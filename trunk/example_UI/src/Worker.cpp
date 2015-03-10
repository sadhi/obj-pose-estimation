/*
 * Worker.cpp
 *
 *  Created on: 23 jan. 2015
 *      Author: sadhi
 */

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

#include <iostream>
#include <opencv2/core/core.hpp>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <opencv2/calib3d/calib3d.hpp>

#include "opencv2/videoio/videoio_c.h"

// wxwidgets includes
#include "wx/wxprec.h"
#include "wx/thread.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

// main header include
#include "Worker.h"
using namespace cv;
using namespace std;

Worker::Worker(MyFrame *frame) : wxThread( ){
	// TODO Auto-generated constructor stub
	m_pFrame = frame;
	pct = new pictureCam_thorlabs((HWND) (m_pFrame->getLiveWindow()->GetHWND()));
	// start life
	m_bLife = 0;
//	pct->render();
//	m_pFrame->getLiveWindow()->Update();
	return;
}

Worker::~Worker() {
	// TODO Auto-generated destructor stub
	pct = NULL;
}

////////////////////////////////////////////////////////////////////
// Method:	On Exit
// Class:	CwxopencvWorker
// Purpose:	do soemthing on exit
// Input:	nothing
// Output:	nothing
////////////////////////////////////////////////////////////////////
void Worker::OnExit( )
{
	// destroy - clean my place
	pct = NULL;
	m_pFrame->setWorker(NULL);
	m_pFrame = NULL;
	cv_image = NULL;
	delete m_pFrame;
	delete pct;
	delete cv_image;
}

void Worker::calcHist(Mat *src)
{
	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat g_hist;

	/// Compute the histograms:
	cv::calcHist( src, 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );

	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 200;
	int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	/// Draw for each channel
	for( int i = 1; i < histSize; i++ )
	{
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
				Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
				Scalar( 0, 255, 0), 2, 8, 0  );
	}
	/// Display
	//TODO: display in gui
	cvtColor(histImage, histImage, COLOR_BGR2RGB);
	m_pFrame->getHistWindow()->DrawCam(&histImage);
//	imshow("histogram", histImage );
}

////////////////////////////////////////////////////////////////////
// Method:		Entry
// Class:		CwxopencvWorker
// Purpose:		the main executable body of my thread
// Input:		nothing
// Output:		void pointer
////////////////////////////////////////////////////////////////////
void *Worker::Entry( )
{
	cout<<"I am in the entry"<<endl;

	Mat imgMat, element, grayImg;
	CvMat im;
	cv_image = cvCreateImageHeader(Size(1280,1024), IPL_DEPTH_8U, 3);

	if( pct->IsInit() )
	{
		//set up matrices for storage
//		Mat distortion = Mat::zeros(5, 1, CV_64F);
//		Mat intrinsics = Mat::eye(3, 3, CV_64F);

		distortion = (Mat_<double>(5,1) << 7.2914041963856420e+000, 2.0979596618049214e+000, -2.3690347009888141e-001, 5.1098820884635382e-002, 1.4682521220954941e-003 );
		intrinsics =  (Mat_<double>(3,3) << 2.0529114902590052e+004, 0., 6.3928941571612074e+002, 0.,
				2.5657205522615815e+004, 5.1836001554552104e+002, 0., 0., 1.) ;

		m_bLife = 1;
	}
	else
	{
		if(pct->OpenCamera())
		{
			//set up matrices for storage
			distortion = Mat::zeros(5, 1, CV_64F);
			intrinsics = Mat::eye(3, 3, CV_64F);

			distortion = (Mat_<double>(5,1) << 7.2914041963856420e+000, 2.0979596618049214e+000, -2.3690347009888141e-001, 5.1098820884635382e-002, 1.4682521220954941e-003 );
			intrinsics =  (Mat_<double>(3,3) << 2.0529114902590052e+004, 0., 6.3928941571612074e+002, 0.,
					2.5657205522615815e+004, 5.1836001554552104e+002, 0., 0., 1.) ;
			m_bLife = 1;
		}
	}
	////////////////////////////////////////////////////////////////
	// Start Life Cycle
	////////////////////////////////////////////////////////////////
	// loop as long as flag m_bLife = 1
	while( m_bLife )
	{
		if( TestDestroy( ) == 1 )
			break;

		pct->render();
		//		cout<<"so slow ..."<<endl;
		/*
		 * Thorlabs does not support opencv so we need to do some of this ourselves
		 * convert image from buffer to an image we can actually use
		 */
		imgMat = Mat(1024, 1280, CV_8UC3, pct->getPcImageMemory());
//		imshow("original", imgMat);
		cvtColor(imgMat, grayImg, COLOR_BGR2GRAY);

		calcHist(&grayImg);

		//		detailEnhance(grayImg,grayImg);	//opencv-3.0.0 beta function
		//TODO: Display in Gui
		cvtColor(grayImg, imgMat, COLOR_GRAY2RGB);
		m_pFrame->getGrayWindow()->DrawCam(&imgMat);
//		if(!m_pFrame->getGrayWindow()->isRunning())
//			m_pFrame->getGrayWindow()->toggleRunning();
//		imshow("result", grayImg);
//		waitKey(1);
	}
	pct->ExitCamera();

    return NULL;
}
