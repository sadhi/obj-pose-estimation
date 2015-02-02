//============================================================================
// Name        : pictureCam.cpp
// Author      : Bas Koevoets
// Version     : v0.1
// Copyright   : Your copyright notice
// Description : Camera/beamer demo application
//============================================================================

#include <windows.h>
#include <unistd.h>
#include <iostream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int exposure = 1;
int index = 0;
bool prfmCap = false;

Mat imgOriginal, pattern, blanc, dark;

void setExposure(int value)
{
	switch (value) {
	case 0: exposure = 1; break;
	case 1: exposure = 2; break;
	case 2: exposure = 5; break;
	case 3: exposure = 10; break;
	case 4: exposure = 20; break;
	case 5: exposure = 39; break;
	case 6: exposure = 78; break;
	case 7: exposure = 156; break;
	case 8: exposure = 312; break;
	case 9: exposure = 625; break;
	case 10: exposure = 1250; break;
	case 11: exposure = 1250; break;
	case 12: exposure = 2500; break;
	case 13: exposure = 5000; break;
	case 14: exposure = 10000; break;
	}
}

void showBlanc()
{
	imshow("Image View", blanc);
//	imshow("Image View", dark);
}

int main() {
	VideoCapture cap(1); //capture the video from 0=webcam, 1=thorlabs camera
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1024);
	if ( !cap.isOpened() )  // if not success, exit program
	{
		cout << "Cannot open the camera" << endl;
		return -1;
	}
	bool running = true;
	Mat result, temp1, temp2;
	cap.read(imgOriginal);

	int w = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int h = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "You have " << GetSystemMetrics(SM_CMONITORS) << " monitor(s) connected\n"<< w<<"x"<<h << endl;

	/*
	 * check if there is more than 1 display
	 * if so than display the window on the display to the right of you primary display
	 */
//	if(GetSystemMetrics(SM_CMONITORS)>1)
//	{
//		namedWindow( "Image View", WINDOW_AUTOSIZE );// Create a window for display
//		namedWindow( "Result View", WINDOW_AUTOSIZE );// Create a window for display
//		namedWindow( "View1", WINDOW_AUTOSIZE );
//		namedWindow( "View2", WINDOW_AUTOSIZE );
//		cvMoveWindow("Image View", GetSystemMetrics(SM_CXSCREEN), 200);
//
//		//		 cvMoveWindow("Image View", 0, 0);
//		pattern = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/chessboard.jpg", 1);	//TODO: I should do this dynamically like in the example
//		blanc = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/blanc.jpg", 1);
//		dark = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/dark.jpg", 1);
////		circle = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/squared_circle.jpg", 1);
//		//		 resize(pattern, pattern, Size(1366,768));
//		imshow("Image View", pattern);
//	}
//	else
//	{
//		cout << "make sure you have a second monitor (or beamer) connected " << endl;
//		cout << "and that it is placed to the right of your primary display " << endl;
//		cout << "now quitting the application" << endl;
//		return -1;
//	}
	namedWindow( "Original", WINDOW_AUTOSIZE );
	cout << "press the spacebar to capture an image"<< endl;
//	cout << "CV_CAP_PROP_EXPOSURE: "<< cap.get(CV_CAP_PROP_EXPOSURE) << endl;
//	cout << "CV_CAP_PROP_AUTO_EXPOSURE: "<< cap.get(CV_CAP_PROP_AUTO_EXPOSURE) << endl;
//	cout << "CV_CAP_PROP_GAIN : "<< cap.get(CV_CAP_PROP_GAIN ) << endl;
//	cout << "CV_CAP_PROP_SATURATION : "<< cap.get(CV_CAP_PROP_SATURATION ) << endl;
//	cout << "CV_CAP_PROP_BRIGHTNESS : "<< cap.get(CV_CAP_PROP_BRIGHTNESS ) << endl;
//	cout << "CV_CAP_PROP_CONTRAST  : "<< cap.get(CV_CAP_PROP_CONTRAST ) << endl;
	//application loop
	while(running)
	{
		cap.read(imgOriginal); // read a new frame from video
		imshow("Original", imgOriginal); //show the original image

		if(prfmCap)
		{
			cap.read(temp2);
			imshow("View2", temp2);			//show the captured image
			result = temp2 - temp1;
			result += result;
			imshow("Result View", result);
			imshow("Image View", pattern);
			prfmCap = false;
		}

		cap.read(imgOriginal); // read a new frame from video
		imshow("Original", imgOriginal); //show the original image

		switch(waitKey(1))
		{
		case 27:	//esc
			cout << "Esc key is pressed by user." << endl;
			cout << "Closing application." << endl;
			running = false;
			break;
		case 32:	//space
			cap.read(temp1);				//normal image (aka 'pattern')
			showBlanc();					//show the blanc image
			imshow("View1", temp1);			// display what was captured
			waitKey(20);
//			cap.read(temp2);				// try to capture the blanc image from the scene
//			waitKey(4);
//			imshow("View2", temp2);			//show the captured image
			prfmCap = true;
			break;
		default:
			break;
		}
	}
	return 0;
}

