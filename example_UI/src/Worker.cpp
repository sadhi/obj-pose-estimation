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
	m_pFrame = frame;
	pct = new pictureCam_thorlabs((HWND) (m_pFrame->getLiveWindow()->GetHWND()));
	// start life
	m_bLife = 0;
//	pct->render();
//	m_pFrame->getLiveWindow()->Update();
	return;
}

Worker::~Worker() {
	pct = NULL;
}

////////////////////////////////////////////////////////////////////
// Method:	On Exit
// Class:	Worker
// Purpose:	clean up on exit
// Input:	nothing
// Output:	nothing
////////////////////////////////////////////////////////////////////
void Worker::OnExit( )
{
	// destroy - clean my place
	m_pFrame->SetStatusText("Worker shutting down");
	pct->ExitCamera();
	pct = NULL;
	m_pFrame->setWorker(NULL);
	m_pFrame = NULL;
	cv_image = NULL;
	delete m_pFrame;
	delete pct;
	delete cv_image;
}

/*
 * this function calculates and draws a histogram based on the supplied image
 */
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

	// Draw the histogram
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
	cvtColor(histImage, histImage, COLOR_BGR2RGB);
	m_pFrame->getHistWindow()->DrawCam(&histImage);
}

void Worker::stop()
{
	m_bLife = 1;
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

		distortion = (Mat_<double>(5,1) << 7.2914041963856420e+000, 2.0979596618049214e+000, -2.3690347009888141e-001, 5.1098820884635382e-002, 1.4682521220954941e-003 );
		intrinsics =  (Mat_<double>(3,3) << 2.0529114902590052e+004, 0., 6.3928941571612074e+002, 0.,
				2.5657205522615815e+004, 5.1836001554552104e+002, 0., 0., 1.) ;

		m_bLife = 1;
		//Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
		r = new Rect(0, 512-m_pFrame->getSlider()->GetValue(), 1280, 512+m_pFrame->getSlider()->GetValue());
		m_pFrame->SetStatusText("starting calibration");
		calibrate();
		m_pFrame->SetStatusText("calibration complete, start measuring");
	}
	else
	{
		if(pct->OpenCamera())
		{
			//set up matrices for storage
			distortion = (Mat_<double>(5,1) << 7.2914041963856420e+000, 2.0979596618049214e+000, -2.3690347009888141e-001, 5.1098820884635382e-002, 1.4682521220954941e-003 );
			intrinsics =  (Mat_<double>(3,3) << 2.0529114902590052e+004, 0., 6.3928941571612074e+002, 0.,
					2.5657205522615815e+004, 5.1836001554552104e+002, 0., 0., 1.) ;

			m_bLife = 1;
			r = new Rect(0, 512-m_pFrame->getSlider()->GetValue(), 1280, 512+m_pFrame->getSlider()->GetValue());
			m_pFrame->SetStatusText("starting calibration");
			calibrate();
			m_pFrame->SetStatusText("calibration complete, start measuring");
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
		/*
		 * Thorlabs does not support opencv so we need to do some of this ourselves
		 * convert image from buffer to an image we can actually use
		 */
		imgMat = Mat(1024, 1280, CV_8UC3, pct->getPcImageMemory());
		cvtColor(imgMat, grayImg, COLOR_BGR2GRAY);

		calcHist(&grayImg);

		//		detailEnhance(grayImg,grayImg);	//opencv-3.0.0 beta function
		cvtColor(grayImg, imgMat, COLOR_GRAY2RGB);
		m_pFrame->getGrayWindow()->DrawCam(&imgMat);
//		imshow("test", imgMat);
//		waitKey(1);
	}
//	pct->ExitCamera();

    return NULL;
}

void Worker::calibrate()
{

	Mat imgMat = Mat(1024, 1280, CV_8UC3, pct->getPcImageMemory());
	cvtColor(imgMat, imgMat, COLOR_BGR2GRAY);

	calcHist(&imgMat);

	//		detailEnhance(grayImg,grayImg);	//opencv-3.0.0 beta function
	cvtColor(imgMat, imgMat, COLOR_GRAY2RGB);
	m_pFrame->getGrayWindow()->DrawCam(&imgMat);
	Mat rMat1 = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));

	wxMessageBox("Change the angle of the x-rotation by ~1 degree (or ~1mm).\nClick 'OK' when you are done.",
			"Action required",
			wxOK | wxICON_INFORMATION,
			m_pFrame);

	Mat rMat2 = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));

	Mat a = (Mat_<double>(1,3) <<	rMat1.at<double>(2,0), rMat1.at<double>(2,1), rMat1.at<double>(2,2));
	Mat b = (Mat_<double>(1,3) <<	rMat2.at<double>(2,0), rMat2.at<double>(2,1), rMat2.at<double>(2,2));

//	Mat a = (Mat_<double>(1,3) <<	-0.8230065775088815, 0.01852642709937535, -0.5677296406530568);
//	Mat b = (Mat_<double>(1,3) <<	-0.8205612951246357, 0.03776898284020541, -0.5703092712542877);

	Mat u1 = pct->calculateU(a, b);
	cout<<"a = " <<a<<endl;
	cout<<"b = " <<b<<endl;
	cout<<"\n"<<endl;

	double t = 0.5 * (u1.at<double>(0,0) + u1.at<double>(1,1) + u1.at<double>(2,2) - 1);
	double theta = acos(t ) * (180/M_PI);	//theta = arcos(1/2[A11 + A22 + A33 - 1])

	double ex1 = (u1.at<double>(2,1) - u1.at<double>(1,2))/(2*sin(theta * M_PI / 180.0));
	double ex2 = (u1.at<double>(0,2) - u1.at<double>(2,0))/(2*sin(theta * M_PI / 180.0));
	double ex3 = (u1.at<double>(1,0) - u1.at<double>(0,1))/(2*sin(theta * M_PI / 180.0));

	rMat2 = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));
	a = (Mat_<double>(1,3) <<	rMat2.at<double>(2,0), rMat2.at<double>(2,1), rMat2.at<double>(2,2));
//	a = (Mat_<double>(1,3) <<	-0.8205612951246357, 0.03776898284020541, -0.5703092712542877);

	wxMessageBox("Change the angle of the y-rotation by ~1 degree (or ~1mm).\nClick 'OK' when you are done.",
			"Action required",
			wxOK | wxICON_INFORMATION,
			m_pFrame);

	rMat1 = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));
	b = (Mat_<double>(1,3) <<	rMat1.at<double>(2,0), rMat1.at<double>(2,1), rMat1.at<double>(2,2));
//	b = (Mat_<double>(1,3) <<	0.8504392751821623, -0.01428064628263098, -0.5258793610414736);

	Mat u2 = pct->calculateU(a, b);
	cout<<"a = " <<a<<endl;
	cout<<"b = " <<b<<endl;
	cout<<"\n"<<endl;

	t = 0.5 * (u2.at<double>(0,0) + u2.at<double>(1,1) + u2.at<double>(2,2) - 1);
	theta = acos(t )* (180/M_PI) ;	//theta = arcos(1/2[A11 + A22 + A33 - 1])

	double ey1 = (u2.at<double>(2,1) - u2.at<double>(1,2))/(2*sin(theta * M_PI / 180.0));
	double ey2 = (u2.at<double>(0,2) - u2.at<double>(2,0))/(2*sin(theta * M_PI / 180.0));
	double ey3 = (u2.at<double>(1,0) - u2.at<double>(0,1))/(2*sin(theta * M_PI / 180.0));

	rMat1 = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));
	a = (Mat_<double>(1,3) <<	rMat1.at<double>(2,0), rMat1.at<double>(2,1), rMat1.at<double>(2,2));
//	a = (Mat_<double>(1,3) <<	0.8504392751821623, -0.01428064628263098, -0.5258793610414736);

	wxMessageBox("Change the angle of the z-rotation by ~1 degree (or ~1mm).\nClick 'OK' when you are done.",
			"Action required",
			wxOK | wxICON_INFORMATION,
			m_pFrame);

	rMat2 = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));
	b = (Mat_<double>(1,3) <<	rMat2.at<double>(2,0), rMat2.at<double>(2,1), rMat2.at<double>(2,2));
//	b = (Mat_<double>(1,3) <<	-0.8344756408017202, 0.0300688893871407, -0.5502238333620073);

	Mat u3 = pct->calculateU(a, b);
	cout<<"a = " <<a<<endl;
	cout<<"b = " <<b<<endl;
	cout<<"\n"<<endl;

	t = 0.5 * (u3.at<double>(0,0) + u3.at<double>(1,1) + u3.at<double>(2,2) - 1) ;
	theta = acos( t) * (180.0 / M_PI);	//theta = arccos(1/2[A11 + A22 + A33 - 1])

	double ez1 = (u3.at<double>(2,1) - u3.at<double>(1,2))/(2*sin(theta * M_PI / 180.0));
	double ez2 = (u3.at<double>(0,2) - u3.at<double>(2,0))/(2*sin(theta * M_PI / 180.0));
	double ez3 = (u3.at<double>(1,0) - u3.at<double>(0,1))/(2*sin(theta * M_PI / 180.0));

	Mat uCamStage = (Mat_<double>(3,3) << ex1, ey1, ez1,
			ex2, ey2, ez2,
			ex3, ey3, ez3);

//	cout<<"( "<<uCamStage.at<double>(0,0)<<", "<< uCamStage.at<double>(0,1)<<", "<< uCamStage.at<double>(0,2)<<";\n"<<
//			uCamStage.at<double>(1,0)<<", "<< uCamStage.at<double>(1,1)<<", "<< uCamStage.at<double>(1,2)<<";\n"<<
//			uCamStage.at<double>(2,0)<<", "<< uCamStage.at<double>(2,1)<<", "<< uCamStage.at<double>(2,2)<<";)\n"<<endl;
	cout<<"uCamStage = "<<uCamStage <<endl;
	Mat uCS1 = uCamStage.inv();
	cout<<"\nuCamStage = "<<uCamStage <<endl;
	cout<<"\nuCS1 = "<<uCS1 <<endl;

	cout<<"\nnew stuff comming"<<endl;
	/*	//figure this out in code, use ROI i.c.w. the slider
		detect object 1 -> a
		Mat a1 = uCamStage * a
		detect object 2 -> b
		Mat b1 = uCamStage * b
		Mat uStage = pct->calculateU(a1, b1);
	 */

	//test data
	rMat1 = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));
	a = (Mat_<double>(1,3) <<	rMat1.at<double>(2,0), rMat1.at<double>(2,1), rMat1.at<double>(2,2));
//	a = (Mat_<double>(1,3) <<	-0.8230065775088815, 0.01852642709937535, -0.5677296406530568);
//	Mat a1 = a*uCamStage;
//	cout<<"a = " <<a1<<endl;
//	cout<<"\nshould be: "<<uCamStage.at<double>(0,0)*a.at<double>(0,0) + uCamStage.at<double>(0,1)*a.at<double>(0,0) + uCamStage.at<double>(0,2)*a.at<double>(0,0)<<";\n"<<
//			uCamStage.at<double>(1,0)*a.at<double>(0,1) + uCamStage.at<double>(1,1)*a.at<double>(0,1) + uCamStage.at<double>(1,2)*a.at<double>(0,1)<<";\n"<<
//			uCamStage.at<double>(2,0)*a.at<double>(0,2) + uCamStage.at<double>(2,1)*a.at<double>(0,2) + uCamStage.at<double>(2,2)*a.at<double>(0,2)<<";\n"<<endl;
	Mat a1 = (Mat_<double>(1,3)<<uCS1.at<double>(0,0)*a.at<double>(0,0) + uCS1.at<double>(0,1)*a.at<double>(0,0) + uCS1.at<double>(0,2)*a.at<double>(0,0),
						uCS1.at<double>(1,0)*a.at<double>(0,1) + uCS1.at<double>(1,1)*a.at<double>(0,1) + uCS1.at<double>(1,2)*a.at<double>(0,1),
						uCS1.at<double>(2,0)*a.at<double>(0,2) + uCS1.at<double>(2,1)*a.at<double>(0,2) + uCS1.at<double>(2,2)*a.at<double>(0,2));
	cout<<"a = " <<a1<<endl;

	wxMessageBox("Change the location of the chip.\nClick 'OK' when you are done.",
			"Action required",
			wxOK | wxICON_INFORMATION,
			m_pFrame);

	rMat2 = pct->calculateRotationMatrix(*r);
//	b = (Mat_<double>(1,3) <<	-0.8205612951246357, 0.03776898284020541, -0.5703092712542877);		//enkel 1mm over x rot
//	b = (Mat_<double>(1,3) <<	-0.8344756408017202, 0.0300688893871407, -0.5502238333620073);		//1mm over x,y&z rot
	b = (Mat_<double>(1,3) <<	rMat2.at<double>(2,0), rMat2.at<double>(2,1), rMat2.at<double>(2,2));
	Mat b1 = (Mat_<double>(1,3)<<uCS1.at<double>(0,0)*b.at<double>(0,0) + uCS1.at<double>(0,1)*b.at<double>(0,0) + uCS1.at<double>(0,2)*b.at<double>(0,0),
							uCS1.at<double>(1,0)*b.at<double>(0,1) + uCS1.at<double>(1,1)*b.at<double>(0,1) + uCS1.at<double>(1,2)*b.at<double>(0,1),
							uCS1.at<double>(2,0)*b.at<double>(0,2) + uCS1.at<double>(2,1)*b.at<double>(0,2) + uCS1.at<double>(2,2)*b.at<double>(0,2));
	cout<<"b = " <<b1<<endl;
	Mat uStage = pct->calculateU(a, b);

	t = uStage.at<double>(0,2);
	thetaY = asin(fmod(t,1))*180/M_PI;
	t = (fmod(uStage.at<double>(2,2),1)/cos(thetaY*M_PI/180));
	thetaX = acos(t)*180/M_PI;
	t = (fmod(uStage.at<double>(0,0),1)/cos(thetaY*M_PI/180));
	thetaZ = acos(t)*180/M_PI;

	std::ostringstream s;
	s << "" << thetaX ;
	m_pFrame->setX1Txt(s.str());

	s.str("");
	s.clear();
	s << "" << thetaY ;
	m_pFrame->setY1Txt(s.str());

	s.str("");
	s.clear();
	s << "" << thetaZ ;
	m_pFrame->setZ1Txt(s.str());

	//alternative from: http://planning.cs.uiuc.edu/node103.html or http://nghiaho.com/?page_id=846
	thetaY = atan2(-uStage.at<double>(2,0),sqrt (uStage.at<double>(2,1)*uStage.at<double>(2,1) + uStage.at<double>(2,2)*uStage.at<double>(2,2))) * 180.0 / M_PI;
	thetaX = atan2(uStage.at<double>(2,1),uStage.at<double>(2,2)) * 180.0 / M_PI;
	thetaZ = atan2(uStage.at<double>(1,0),uStage.at<double>(0,0)) * 180.0 / M_PI;

	s.str("");
	s.clear();
	s << "" << thetaX ;
	m_pFrame->setX2Txt(s.str());

	s.str("");
	s.clear();
	s << "" << thetaY ;
	m_pFrame->setY2Txt(s.str());

	s.str("");
	s.clear();
	s << "" << thetaZ ;
	m_pFrame->setZ2Txt(s.str());
//	Mat e = (Mat_<double>(3,3) << 0,-e3,e2, e3,0,-e1, -e2,e1,0);
//	cout<<"e = " <<e<<endl;

}

Rect* Worker::getROI()
{
	return r;
}
