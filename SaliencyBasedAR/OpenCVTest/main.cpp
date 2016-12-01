#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>

#ifdef __APPLE__ // Not really for apple, just for Brandon's specific build...
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#else
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

using namespace cv;
using namespace std;

struct ItemLocation
{
	int lef;
	int top;
	int rig;
	int bot;
};

// A class for draw label fram
struct ObjectLabel
{
	string label;
	ItemLocation location;
	double averIntensity;	

	// Display properties
	static const double DEFAULT_FONT_SCALE = 0.5;
	static const double DEFAULT_FONT_THICKNESS = 0.5;
	static const double DISPLAY_THRESHOLD = 2; // Must be >= 2 to display

	int fontFace;
	double fontScale; // Word size
	double thickness;
	double textWidth;
	double textHeight;
    double alpha;
	int baseline;
	int r, g, b; // color for rect and line

	// Variables for specific experiments
	int ranking; // A parameter to control whether hide or show the labels
	bool resizingEnabled;
	bool boldnessEnabled;
	bool transparencyEnabled;
	bool forceDisplay;

	ObjectLabel() 
	{
		fontFace = FONT_HERSHEY_SIMPLEX;
		fontScale = DEFAULT_FONT_SCALE;
		thickness = DEFAULT_FONT_THICKNESS;
	}

    void doRankBasedResizing()
    {
    	fontScale = ranking == 0 ? 0.0f : 0.4f + (0.1f * ranking);
        updateSize();
    }

    void doRankBasedBoldness()
    {
    	thickness = ranking == 0 ? 0.0f : 0.4f + (0.2f * ranking);
    }

    void doRankBasedTransparency()
    {
    	alpha = ranking == 0 ? 0.0f : 0.2f + (ranking - 1) * 0.1f;
    }

    void updateSize()
    {
        Size textSize =
            getTextSize(label, fontFace, fontScale, thickness, &baseline);
        textWidth = textSize.width;
        textHeight = textSize.height;
    }

    void setAvgIntensity(double averIntensity)
    {
    	this->averIntensity = averIntensity;
    	ranking = averIntensity / 10;
    }

	// Control the size, color of the label, the size of the rect is determined by the size of words;
	void setLabel(string lab)
	{
		label = lab;
		updateSize();
	}

	void setColor(int ir, int ig, int ib)
	{
		r = ir;
		g = ig;
		b = ib;
	}

    cv::Mat doLabelBasedimgCut(Mat& img)
    {
        Mat roi;
        Point topLeftCorner(location.lef, location.top);
        roi = img(cv::Rect(topLeftCorner - Point(0, textHeight),
        		  topLeftCorner + Point(textWidth, baseline)));
        return roi;
    }

	void drawRect(Mat& img)
	{
		if (transparencyEnabled)
		{
			Mat roi = doLabelBasedimgCut(img);
	        Mat color(roi.size(), CV_8UC3, cv::Scalar(b, g, r));
	        addWeighted(color, alpha, roi, 1 - alpha, 0, roi, -1);
	    }
	   	else
	   	{
			Point topLeftCorner(location.lef, location.top);
			Point center = topLeftCorner + Point(textWidth / 2.0, textHeight / 2.0);
			rectangle(img, topLeftCorner - Point(0, textHeight),
					  topLeftCorner + Point(textWidth, baseline), Scalar(b, g, r), -1, 1);
		}
	}

	void drawLine(Mat& img)
	{
		Point topLeftCorner(location.lef, location.top);
		Point objectCenter = topLeftCorner + Point((location.rig - location.lef) / 2.0,
												   (location.bot - location.top) / 2.0);
		Point labelCenter = topLeftCorner + Point(textWidth / 2.0, baseline);
		line(img, objectCenter, labelCenter, Scalar(b, g, r), 2, 8);
	}

	void drawText(Mat& img)
	{
		// then put the text itself
        Point topLeftCorner(location.lef, location.top);
		putText(img, label, topLeftCorner, fontFace, fontScale,
				Scalar(255, 255, 255), thickness, 8);
	}

	void display(Mat& img)
	{
		if (ranking >= DISPLAY_THRESHOLD || forceDisplay)
		{
			// final we could make a case function...based on different value the ranking retures
			setColor(0, 0, 255);
			drawRect(img);
			drawText(img);
			drawLine(img);
		}
	}

	void enableResizing(bool val)
	{
		resizingEnabled = val;
		if (resizingEnabled)
		{
			doRankBasedResizing();
		} 
		else 
		{
			fontScale = DEFAULT_FONT_SCALE;
			updateSize();
		}
	}

	void enableBoldness(bool val)
	{
		boldnessEnabled = val;
		if (boldnessEnabled) 
		{
			doRankBasedBoldness();
		}
		else
		{
			thickness = DEFAULT_FONT_THICKNESS;
			updateSize();
		}
	}
    
	void enableTransparency(bool val)
    {
    	transparencyEnabled = val;
    	if (transparencyEnabled) 
    	{
    		doRankBasedTransparency();
    	}
    }

	void registerMouseInput(int x, int y)
	{
		forceDisplay = x > location.lef && x < location.rig &&
					   y > location.top && y < location.bot;
	}
};

void readObjectLabels(string filePath, vector<ObjectLabel>& objects, const Mat& salmap)
{
	ifstream readFileStream;
	readFileStream.open(filePath);
	if (!readFileStream)
	{
		cout << "While opening a file an error is encountered" << endl;
	}
	else
	{
		cout << "File is successfully opened" << endl;
		while (!readFileStream.eof())
		{
			string lab, lef, top, rig, bot;
			Scalar intensity = 0;
			double averIntensity = 0.0;

			ObjectLabel obj;
			readFileStream >> lab >> lef >> top >> rig >> bot;
			if (!(lab.empty() && lef.empty() && top.empty() && rig.empty() && bot.empty()))
			{
				obj.setLabel(lab.substr(4));
				obj.location.lef = atoi(lef.substr(4).c_str());
				obj.location.top = atoi(top.substr(4).c_str());
				obj.location.rig = atoi(rig.substr(4).c_str());
				obj.location.bot = atoi(bot.substr(4).c_str());

				int t = 0;
				for (int j = obj.location.lef; j < obj.location.rig; j++)
				{
					for (int i = obj.location.top; i < obj.location.bot; i++)
					{
						intensity.val[0] = intensity.val[0] + salmap.at<uchar>(Point(j, i));
						t++;
					}
				}
				averIntensity = intensity.val[0] / t;
                obj.setAvgIntensity(averIntensity);
				cout << "Debug: ItemLocation Lab: " << obj.label 
				   	 << ", lef: " << obj.location.lef 
				   	 << ", top: " << obj.location.top 
				   	 << ", rig: " << obj.location.rig 
				   	 << ", bot: " << obj.location.bot 
				   	 << ", averIntensity:" << averIntensity << endl;
				objects.push_back(obj);
			}
		}
	}
}

// ************ Global State **********
int mouseX = 0;
int mouseY = 0;

void mouseFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_MOUSEMOVE)
	{
		mouseX = x;
		mouseY = y;
	}
}

int main(int argc, char** argv){
	string file = "D:\\test1.jpg";
	string salmapFile = "D:\\test1_msss.jpg.jpg";
	string predictionsFile = "D:\\test1_predictions.txt";

    if (argc == 2) {
        string item_name = argv[1];
        file = item_name + ".jpg";
	    salmapFile = item_name + "_msss.jpg.jpg";
	    predictionsFile = item_name + "_predictions.txt";
    }

	// Read saliency map in grayscale
	Mat salmap = imread(salmapFile, 0); 
	vector<ObjectLabel> objs;
	readObjectLabels(predictionsFile, objs, salmap);

    Mat original = imread(file);

    // Display loop
    Mat img;
	namedWindow("original image", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("original image", &mouseFunc, NULL);
	do
	{
		original.copyTo(img);
		for (ObjectLabel& obj : objs)
		{
			obj.enableResizing(false);
			obj.enableBoldness(false);
			obj.enableTransparency(true);
			obj.registerMouseInput(mouseX, mouseY);
	        obj.display(img);
	        imshow("original image", img);   
		}
	} while(waitKey(16) == -1);

	return 0;
}

