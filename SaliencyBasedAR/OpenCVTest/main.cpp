#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

/// Global variables

Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

/**
* @function CannyThreshold
* @brief Trackbar callback - Canny thresholds input with a ratio 1:3
*/
void CannyThreshold(int, void*)
{
	/// Reduce noise with a kernel 3x3
	blur(src_gray, detected_edges, Size(3, 3));

	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all(0);

	src.copyTo(dst, detected_edges);
	imshow(window_name, dst);
}


void mouseEvent(int event, int x, int y, int flags, void* param)
{
	IplImage* pic = (IplImage*) param;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		cout << "the point is:" << x << "," << y << endl;
	}
}

int main(){
	string text = "OpenCVtest";
	//string file = "D:\\fig12.4.jpg";//Test image file
	string file = "C:\\image1.png";//Test image file
	int fontFace = FONT_HERSHEY_SIMPLEX;
	double fontScale = 0.5;
	double alpha = 0.7;
	int thickness = 1;

	//Mat img(600, 800, CV_8UC3, Scalar::all(0));
	Mat img = imread(file);
	Mat copy;
	img.copyTo(copy);
	src = imread(file);

	int baseline = 0;
	Size textSize = getTextSize(text, fontFace,
		fontScale, thickness, &baseline);
	baseline += thickness;

	// center the text
	Point textOrg((img.cols - textSize.width) / 2,
		(img.rows + textSize.height) / 2);
	// draw the box
	rectangle(copy, textOrg + Point(0, baseline),
		textOrg + Point(textSize.width, -textSize.height),
		Scalar(0, 0, 0), -1, 1);

	line(copy, Point(0, 0), textOrg + Point(0, baseline), Scalar(0, 0, 0), 2, 8);

	addWeighted(copy, alpha, img, 1-alpha ,0, img);

	// ... and the baseline first
	/*line(img, textOrg + Point(0, thickness),
		textOrg + Point(textSize.width, thickness),
		Scalar(0, 0, 255));*/

	// then put the text itself
	putText(img, text, textOrg, fontFace, fontScale,
		Scalar::all(255), thickness, 8);
	//Read image from file
	IplImage *pic = cvLoadImage("file",1);
	namedWindow("res");

	//set the callback function for any mouse event
	cvSetMouseCallback("res",mouseEvent,&pic);
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
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	/// Create a Trackbar for user to enter threshold
	createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);

	/// Show the image
	CannyThreshold(0, 0);

	//show the image
	imshow("res", img);

	waitKey();

	return 0;
}
