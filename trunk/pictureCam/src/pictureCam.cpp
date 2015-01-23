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

int main() {
	VideoCapture cap(1); //capture the video from 0=webcam, 1=thorlabs camera
	if ( !cap.isOpened() )  // if not success, exit program
	{
		cout << "Cannot open the camera" << endl;
		return -1;
	}
	bool running = true;
	Mat imgOriginal;
	Mat pattern;
	Mat blanc;
	Mat dark;
	Mat circle;
	Mat result, temp1, temp2;

	cout << "You have " << GetSystemMetrics(SM_CMONITORS) << " monitor(s) connected" << endl;

	 /*
	  * check if there is more than 1 display
	  * if so than display the window on the display to the right of you primary display
	  */
	 if(GetSystemMetrics(SM_CMONITORS)>1)
	 {
		 namedWindow( "Image View", WINDOW_AUTOSIZE );// Create a window for display
		 cvMoveWindow("Image View", GetSystemMetrics(SM_CXSCREEN), 200);
//		 cvMoveWindow("Image View", 0, 0);
		 pattern = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/chessboard.jpg", 1);	//TODO: I should do this dynamically like in the example
		 blanc = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/blanc.jpg", 1);
		 dark = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/dark.jpg", 1);
		 circle = imread("D:/Users/sadhi/workspace/pictureCam/obj-pose-estimation/pictureCam/Debug/squared_circle.jpg", 1);
//		 resize(pattern, pattern, Size(1366,768));
		 imshow("Image View", pattern);
	 }
	 else
	 {
		 cout << "make sure you have a second monitor (or beamer) connected " << endl;
		 cout << "and that it is placed to the right of your primary display " << endl;
		 cout << "now quitting the application" << endl;
		 return -1;
	 }

	//application loop
	while(running)
	{
		bool bSuccess = cap.read(imgOriginal); // read a new frame from video
		if (!bSuccess) //if not success, break loop
		{
			cout << "success? last error: "<< GetLastError() << endl;
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		imshow("Original", imgOriginal); //show the original image
		cout<< "Exposure is currently set to: " << cap.get(CV_CAP_PROP_EXPOSURE) <<endl;
		switch(waitKey(10))
		{
		case 27:	//esc
			cout << "Esc key is pressed by user." << endl;
			cout << "Closing application." << endl;
			running = false;
			break;
		case 13:	//enter
//			cap.set(CV_CAP_PROP_EXPOSURE,-8);
			cap.read(temp1);
			imshow("Image View", blanc);
			cap.read(temp2);

			result = temp1 - temp2;
			imshow("Image View", result);
			break;
		case 32:	//space
			cap.read(temp1);
			imshow("Image View", dark);
			cap.read(temp2);
			result = temp2 + temp1;
			imshow("Image View", result);
			break;
		default:
			imshow("Image View", pattern);
			break;
		}
	}
	return 0;
}
