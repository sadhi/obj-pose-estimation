//============================================================================
// Name        : pictureCam.cpp
// Author      : Bas Koevoets
// Version     : v0.1
// Copyright   : Your copyright notice
// Description : Camera/beamer demo application
//============================================================================

#include "windows.h"
#include <iostream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int main() {
	VideoCapture cap(0); //capture the video from 0=webcam, 1=thorlabs camera
	if ( !cap.isOpened() )  // if not success, exit program
	{
		cout << "Cannot open the camera" << endl;
		return -1;
	}

	Mat imgOriginal;
	Mat pattern;

	cout << "You have " << GetSystemMetrics(SM_CMONITORS) << " monitor(s) connected" << endl;

	 /*
	  * check if there is more than 1 display
	  * if so than display the window on 0,0 on the display to the right of you primary display
	  */
	 if(GetSystemMetrics(SM_CMONITORS)>1)
	 {
		 namedWindow( "Image View", WINDOW_AUTOSIZE );// Create a window for display
		 cvMoveWindow("Image View", GetSystemMetrics(SM_CXSCREEN), 0);
//		 cvMoveWindow("Image View", 0, 0);
		 pattern = imread("D:/Users/sadhi/workspace/pictureCam/Debug/chessboard.jpg", 1);	//TODO: I should do this dynamically like in the example
		 resize(pattern, pattern, Size(1366,768));
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
	while(true)
	{
		bool bSuccess = cap.read(imgOriginal); // read a new frame from video
		if (!bSuccess) //if not success, break loop
		{
			cout << "success? last error: "<< GetLastError() << endl;
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		imshow("Original", imgOriginal); //show the original image

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "Esc key is pressed by user." << endl;
			cout << "Closing application." << endl;
			break;
		}
	}
	return 0;
}
