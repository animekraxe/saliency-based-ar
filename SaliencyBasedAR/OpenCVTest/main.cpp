#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include<fstream>
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

//A class for draw label fram
struct ObjectLabel
{
	string label;
	ItemLocation location;
	double averIntensity;	

	// Display properties
	static const double DEFAULT_FONT_SCALE = 0.5;
	static const double DEFAULT_FONT_THICKNESS = 0.5;

	int fontFace;
	double fontScale; //word size
	double thickness;
	double textWidth;
	double textHeight;
    double alpha;
	int baseline;
	int r, g, b; // color for rect and line

	// Variables for specific experiments
	int ranking; // a parameter to control whether hide or show the labels
	bool resizingEnabled;
	bool boldnessEnabled;

	ObjectLabel() 
	{
		fontFace = FONT_HERSHEY_SIMPLEX;
		fontScale = DEFAULT_FONT_SCALE;
		thickness = DEFAULT_FONT_THICKNESS;
	}

    void doRankBasedResizing()
    {
		double newAlpha = 0.0;
    	if (ranking == 0)
    	{
    		fontScale = 0.0;
    		newAlpha=0;
    	}
    	else
    	{
    		fontScale = 0.4 + (0.1 * ranking);
    		newAlpha=0.2+(ranking-1)*0.1;
    	}
        updateSize();
        setLabelAlpha(newAlpha);
    }

    void doRankBasedBoldness()
    {
    	if (ranking == 0)
    	{
    		thickness = 0.0;
    	} 
    	else 
    	{
    		thickness = 0.4 + (0.2 * ranking);
    	}
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

	//control the size, color of the label, the size of the rect is determined by the size of words;
	void setLabel(string lab)
	{
		label = lab;
		updateSize();
	}
    
    void setLabelAlpha(double labelTransparent)
    {
        alpha = labelTransparent;
    }
    
    void setLabelTransparent(Mat& copy,Mat& img)
    {
        addWeighted(copy, alpha, img, 1 - alpha, 0, img, -1);
    }

	void setColor(int ir, int ig, int ib)
	{
		r = ir;
		g = ig;
		b = ib;
	}

	void drawRect(Mat& copy)
	{
		Point topLeftCorner(location.lef, location.top);
		Point center = topLeftCorner + Point(textWidth / 2.0, textHeight / 2.0);
		rectangle(copy, topLeftCorner - Point(0, textHeight), topLeftCorner + Point(textWidth, baseline),
				  Scalar(b, g, r), -1, 1);
	}

	void drawLine(Mat& copy)
	{
		Point topLeftCorner(location.lef, location.top);
		Point objectCenter = topLeftCorner + Point((location.rig - location.lef) / 2.0,
												   (location.bot - location.top) / 2.0);
		Point labelCenter = topLeftCorner + Point(textWidth / 2.0, baseline);
		line(copy, objectCenter, labelCenter, Scalar(b, g, r), 2, 8);
	}

	void drawText(Mat& img)
	{

		// then put the text itself

        Point topLeftCorner(location.lef, location.top);
		putText(img, label, topLeftCorner, fontFace, fontScale,
				Scalar(255, 255, 255), thickness, 8);
	}

	void display(Mat& copy)
	{
		// final we could make a case function...based on different value the ranking retures
		if (ranking > 1) 
		{
			//int baseline = 0;
			//baseline += thickness;

			setColor(0, 0, 255);
			drawRect(copy);
			drawText(copy);
			drawLine(copy);
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
						//std::cout << "Matrix of image loaded is: " << img.at<uchar>(i, j);
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

/// Global variables

Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int inputRatio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

/**
* @function CannyThreshold
* @brief Trackbar callback - Canny thresholds input with a ratio 1:3
*/
//void CannyThreshold(int, void*)
//{
//	/// Reduce noise with a kernel 3x3
//	blur(src_gray, detected_edges, Size(3, 3));
//
//	/// Canny detector
//	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*inputRatio, kernel_size);
//
//	/// Using Canny's output as a mask, we display our result
//	dst = Scalar::all(0);
//
//	src.copyTo(dst, detected_edges);
//	imshow(window_name, dst);
//}
//
//void mouseEvent(int event, int x, int y, int flags, void* param)
//{
//	IplImage* pic = (IplImage*) param;
//	if (event == CV_EVENT_LBUTTONDOWN)
//	{
//		cout << "the point is:" << x << "," << y << endl;
//	}
//}

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


	Mat salmap = imread(salmapFile, 0); //read saliency map in grayscale
	vector<ObjectLabel> objs;
	readObjectLabels(predictionsFile, objs, salmap);

    Mat img = imread(file);
    Mat copy;
    img.copyTo(copy);
	//src = imread(file);
	for (auto& obj : objs)
	{
		obj.enableResizing(true);
		obj.enableBoldness(true);
        obj.display(copy);
        obj.setLabelTransparent(copy, img);
	}

	namedWindow("original image", CV_WINDOW_AUTOSIZE);
	imshow("original image", img);

	//namedWindow("saliency map", CV_WINDOW_AUTOSIZE);
	//imshow("saliency map", salmap);

	waitKey();

	/*
	// For Edge Detection
	// from main edgemapping
	if (!src.data)
	{
		return -1;
	}

	/// Create a matrix of the same type and size as src (for dst)
	dst.create(src.size(), src.type());

	/// Convert the image to grayscale
	cvtColor(src, src_gray, CV_BGR2GRAY);

	/// Create a window
	//namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	/// Create a Trackbar for user to enter threshold
	//createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);

	/// Show the image
	//CannyThreshold(0, 0);
	*/

	return 0;
}

