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

#include <Eigen>
#include <complex>

#include <opencv2/core/core.hpp>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <opencv2/calib3d/calib3d.hpp>
#include "opencv2/core/eigen.hpp"

#include "opencv2/videoio/videoio_c.h"
#include <LU>

// wxwidgets includes
#include "wx/wxprec.h"
#include "wx/thread.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "Worker.h"
using namespace cv;
using namespace std;
using namespace Eigen;

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

		//test if there was a rotation
		m_b = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));
		determineRotation(m_a, m_b);
		m_a = m_b;
//		imshow("test", imgMat);
		switch(waitKey(1))
		{
//		case 27:	//esc
//			cout << "Esc key is pressed by user." << endl;
//			cout << "Closing application." << endl;
//			running = false;
//			break;
		case 32:	//space
			wxMessageBox("Click 'OK' to continue.",
					"Action required",
					wxOK | wxICON_INFORMATION,
					m_pFrame);
			break;
		default:
			break;
		}
	}
//	pct->ExitCamera();

    return NULL;
}

void Worker::calibrate()
{

	Mat imgMat = Mat(1024, 1280, CV_8UC3, pct->getPcImageMemory());
	cvtColor(imgMat, imgMat, COLOR_BGR2GRAY);

	calcHist(&imgMat);

	cvtColor(imgMat, imgMat, COLOR_GRAY2RGB);
	m_pFrame->getGrayWindow()->DrawCam(&imgMat);
	Mat a = pct->calculateTranslation(Rect(0, 0, r->width, r->y));

	wxMessageBox("Change the angle of the x-rotation by ~1 degree (or ~1mm).\nClick 'OK' when you are done.",
			"Action required",
			wxOK | wxICON_INFORMATION,
			m_pFrame);

	Mat b = pct->calculateTranslation(Rect(0, 0, r->width, r->y));

	wxMessageBox("Change the angle of the y-rotation by ~1 degree (or ~1mm).\nClick 'OK' when you are done.",
			"Action required",
			wxOK | wxICON_INFORMATION,
			m_pFrame);

	Mat c = pct->calculateTranslation(Rect(0, 0, r->width, r->y));

	wxMessageBox("Change the angle of the z-rotation by ~1 degree (or ~1mm).\nClick 'OK' when you are done.",
			"Action required",
			wxOK | wxICON_INFORMATION,
			m_pFrame);

	m_a = m_b = pct->calculateRotationMatrix(Rect(0, 0, r->width, r->y));
	Mat d = m_a.col(3);

	Mat m = (Mat_<double>(3,3) << b.at<double>(0,0) - a.at<double>(0,0), b.at<double>(1,0) - a.at<double>(1,0), b.at<double>(2,0) - a.at<double>(2,0),
			c.at<double>(0,0) - a.at<double>(0,0), c.at<double>(1,0) - a.at<double>(1,0), c.at<double>(2,0) - a.at<double>(2,0),
			d.at<double>(0,0) - a.at<double>(0,0), d.at<double>(1,0) - a.at<double>(1,0), d.at<double>(2,0) - a.at<double>(2,0));

	double a_abs = sqrt(pow(a.at<double>(0,0),2) + pow(a.at<double>(1,0),2) + pow(a.at<double>(2,0),2)) ;
	double b_abs = sqrt(pow(b.at<double>(0,0),2) + pow(b.at<double>(1,0),2) + pow(b.at<double>(2,0),2));
	double c_abs = sqrt(pow(c.at<double>(0,0),2) + pow(c.at<double>(1,0),2) + pow(c.at<double>(2,0),2));
	double d_abs = sqrt(pow(d.at<double>(0,0),2) + pow(d.at<double>(1,0),2) + pow(d.at<double>(2,0),2));
	Mat det = (Mat_<double>(3,1) <<	pow(a_abs,2) - pow(b_abs,2),
			pow(a_abs,2) - pow(c_abs,2),
			pow(a_abs,2) - pow(d_abs,2));
	cout << "det = "<<det <<endl;
	// 2 M * q = D
	// 2 q = M^-1 * D
	// q = 0.5 * M^-1 * D
	rotPoint = 0.5 * m.inv() * det;
	cout << "\ncommon rotation point = " << rotPoint <<endl;

//	std::ostringstream s;
//	s << "" << thetaX ;
//	m_pFrame->setX1Txt(s.str());
//
//	s.str("");
//	s.clear();
//	s << "" << thetaY ;
//	m_pFrame->setY1Txt(s.str());
//
//	s.str("");
//	s.clear();
//	s << "" << thetaZ ;
//	m_pFrame->setZ1Txt(s.str());
//
//	//alternative from: http://planning.cs.uiuc.edu/node103.html or http://nghiaho.com/?page_id=846
//	thetaY = atan2(-uStage.at<double>(2,0),sqrt (uStage.at<double>(2,1)*uStage.at<double>(2,1) + uStage.at<double>(2,2)*uStage.at<double>(2,2))) * 180.0 / M_PI;
//	thetaX = atan2(uStage.at<double>(2,1),uStage.at<double>(2,2)) * 180.0 / M_PI;
//	thetaZ = atan2(uStage.at<double>(1,0),uStage.at<double>(0,0)) * 180.0 / M_PI;
//
//	s.str("");
//	s.clear();
//	s << "" << thetaX ;
//	m_pFrame->setX2Txt(s.str());
//
//	s.str("");
//	s.clear();
//	s << "" << thetaY ;
//	m_pFrame->setY2Txt(s.str());
//
//	s.str("");
//	s.clear();
//	s << "" << thetaZ ;
//	m_pFrame->setZ2Txt(s.str());

//	Mat e = (Mat_<double>(3,3) << 0,-e3,e2, e3,0,-e1, -e2,e1,0);
//	cout<<"e = " <<e<<endl;

}

void Worker::determineRotation(Mat a, Mat b)
{

	Mat a1 = Mat::zeros(4,4,CV_64F);
	Mat b1 = Mat::zeros(4,4,CV_64F);

	Mat a2 = Mat::zeros(4,4,CV_64F);
	Mat b2 = Mat::zeros(4,4,CV_64F);

	Mat a3 = Mat::zeros(4,4,CV_64F);
	Mat b3 = Mat::zeros(4,4,CV_64F);

	//Setting up data
	//first set
	a1.at<double>(0,1) = -(a.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	a1.at<double>(1,0) = a.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0);
	a1.at<double>(0,2) = -(a.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	a1.at<double>(2,0) = a.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	a1.at<double>(0,3) = -(a.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	a1.at<double>(3,0) = a.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	a1.at<double>(1,2) = a.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	a1.at<double>(2,1) = -(a.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	a1.at<double>(1,3) = -(a.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	a1.at<double>(3,1) = a.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	a1.at<double>(2,3) = -(a.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	a1.at<double>(3,2) = a.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0);

	b1.at<double>(0,1) = -(b.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	b1.at<double>(1,0) = b.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0);
	b1.at<double>(0,2) = -(b.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	b1.at<double>(2,0) = b.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	b1.at<double>(0,3) = -(b.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	b1.at<double>(3,0) = b.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	b1.at<double>(1,2) = b.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	b1.at<double>(2,1) = -(b.at<double>(2,0) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	b1.at<double>(1,3) = -(b.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	b1.at<double>(3,1) = b.at<double>(1,0) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	b1.at<double>(2,3) = -(b.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	b1.at<double>(3,2) = b.at<double>(0,0) + a.at<double>(0,3) - rotPoint.at<double>(0,0);

	//second set
	a2.at<double>(0,1) = -(a.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	a2.at<double>(1,0) = a.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0);
	a2.at<double>(0,2) = -(a.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	a2.at<double>(2,0) = a.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	a2.at<double>(0,3) = -(a.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	a2.at<double>(3,0) = a.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	a2.at<double>(1,2) = a.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	a2.at<double>(2,1) = -(a.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	a2.at<double>(1,3) = -(a.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	a2.at<double>(3,1) = a.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	a2.at<double>(2,3) = -(a.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	a2.at<double>(3,2) = a.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0);

	b2.at<double>(0,1) = -(b.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	b2.at<double>(1,0) = b.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0);
	b2.at<double>(0,2) = -(b.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	b2.at<double>(2,0) = b.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	b2.at<double>(0,3) = -(b.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	b2.at<double>(3,0) = b.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	b2.at<double>(1,2) = b.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	b2.at<double>(2,1) = -(b.at<double>(2,1) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	b2.at<double>(1,3) = -(b.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	b2.at<double>(3,1) = b.at<double>(1,1) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	b2.at<double>(2,3) = -(b.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	b2.at<double>(3,2) = b.at<double>(0,1) + a.at<double>(0,3) - rotPoint.at<double>(0,0);

	//third set
	a3.at<double>(0,1) = -(a.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	a3.at<double>(1,0) = a.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0);
	a3.at<double>(0,2) = -(a.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	a3.at<double>(2,0) = a.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	a3.at<double>(0,3) = -(a.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	a3.at<double>(3,0) = a.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	a3.at<double>(1,2) = a.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	a3.at<double>(2,1) = -(a.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	a3.at<double>(1,3) = -(a.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	a3.at<double>(3,1) = a.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	a3.at<double>(2,3) = -(a.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	a3.at<double>(3,2) = a.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0);

	b3.at<double>(0,1) = -(b.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	b3.at<double>(1,0) = b.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0);
	b3.at<double>(0,2) = -(b.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	b3.at<double>(2,0) = b.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	b3.at<double>(0,3) = -(b.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	b3.at<double>(3,0) = b.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	b3.at<double>(1,2) = b.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0);
	b3.at<double>(2,1) = -(b.at<double>(2,2) + a.at<double>(2,3) - rotPoint.at<double>(2,0));
	b3.at<double>(1,3) = -(b.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0));
	b3.at<double>(3,1) = b.at<double>(1,2) + a.at<double>(1,3) - rotPoint.at<double>(1,0);
	b3.at<double>(2,3) = -(b.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0));
	b3.at<double>(3,2) = b.at<double>(0,2) + a.at<double>(0,3) - rotPoint.at<double>(0,0);

	Mat n = (a1.t() * b1) + (a2.t() * b2) + (a3.t() * b3);

	Mat n_com = Mat::zeros(4,4,CV_64FC2);// (Mat_< Vec2f >(4, 4) << n.at<double>(0,0), n.at<double>(0,1), n.at<double>(0,2), n.at<double>(0,3),
	for(int i = 0; i<4; i++)
	{
		for(int j = 0; j<4; j++)
		{
			n_com.at<Vec2d>(i,j)[0] = n.at<double>(i,j);
			n_com.at<Vec2d>(i,j)[1] = 0;
		}
	}

	MatrixXcd eigenN;
	cv2eigen(n_com,eigenN);
	ComplexEigenSolver<MatrixXcd> ces;
	ces.compute(eigenN);
	cout << "The eigenvalues of A are:" << endl << ces.eigenvalues() << endl;

	complex<double> zmax = ces.eigenvalues()[0];
	complex<double> tcom;
	int com_idx[4] = {0,-1,-1,-1};
	int count = 1;

	for (int i=1;i<4;i++) {
		tcom = ces.eigenvalues()[i];
		if (roundf(tcom.real()) > roundf(zmax.real()))
		{
			cout<< tcom.real() <<" > " << zmax.real() <<endl;
			zmax = ces.eigenvalues()[i];
			com_idx[count-1] = -1;
			count = 0;
			com_idx[count] = i;
			count++;
		}
		else if (roundf(tcom.real()) == roundf(zmax.real()))
		{
			cout<<"2 equal numbers"<<endl;
			com_idx[count] = i;
			count++;
		}
		cout<<"numbers are: " << com_idx[0]<< com_idx[1]<< com_idx[2]<<com_idx[3]<<endl;
	}
	cout << "\nThe eigenvectors are:" << endl << ces.eigenvectors();
	double m_w, m_x, m_y, m_z, m_angle;
	cout << "\nThe first eigenvector is:\n"  << ces.eigenvectors().col(com_idx[0]);
	VectorXcd v = ces.eigenvectors().col(com_idx[0]);
	tcom = v(0);
	m_w = tcom.real();//roundf(tcom.real()*1000)/1000;
	m_angle = acos(m_w)* 360/M_PI;//(360/M_PI);
	cout<<"\nangle = "<<m_angle<<endl;
	tcom = v(1);
	m_x = tcom.real();//roundf(tcom.real()*1000)/1000;
	cout<<"x-axis = "<<m_x / sin(m_angle * M_PI / 360)<<endl;
	tcom = v(2);
	m_y = tcom.real();//roundf(tcom.real()*1000)/1000;
	cout<<"y-axis = "<<m_y / sin(m_angle * M_PI / 360)<<endl;
	tcom = v(3);
	m_z = tcom.real();//roundf(tcom.real()*1000)/1000;
	cout<<"z-axis = "<<m_z / sin(m_angle * M_PI / 360)<<endl;
	if(com_idx[1]!=-1 && com_idx[1]>com_idx[0])
	{
		cout << "\nThe second eigenvector is:\n" << ces.eigenvectors().col(com_idx[1]);
		v = ces.eigenvectors().col(com_idx[1]);
		tcom = v(0);
		m_w = tcom.real();//roundf(tcom.real()*1000)/1000;
		m_angle = acos(m_w)* (360/M_PI);
		cout<<"\nangle = "<<m_angle<<endl;
		tcom = v(1);
		m_x = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"x-axis = "<<m_x / sin(m_angle * M_PI / 360)<<endl;
		tcom = v(2);
		m_y = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"y-axis = "<<m_y / sin(m_angle * M_PI / 360)<<endl;
		tcom = v(3);
		m_z = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"z-axis = "<<m_z / sin(m_angle * M_PI / 360)<<endl;
	}
	if(com_idx[2]!=-1)
	{
		cout << "\nThe third eigenvector is:\n" << ces.eigenvectors().col(com_idx[2]);
		v = ces.eigenvectors().col(com_idx[2]);
		tcom = v(0);
		m_w = tcom.real();//roundf(tcom.real()*1000)/1000;
		m_angle = acos(m_w)* (360/M_PI);
		cout<<"\nangle = "<<m_angle<<endl;
		tcom = v(1);
		m_x = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"x-axis = "<<m_x / sin(m_angle * M_PI / 360)<<endl;
		tcom = v(2);
		m_y = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"y-axis = "<<m_y / sin(m_angle * M_PI / 360)<<endl;
		tcom = v(3);
		m_z = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"z-axis = "<<m_z / sin(m_angle * M_PI / 360)<<endl;
	}
	if(com_idx[3]!=-1)
	{
		cout << "\nThe fourth eigenvector is:\n" << ces.eigenvectors().col(com_idx[3]);
		v = ces.eigenvectors().col(com_idx[3]);
		tcom = v(0);
		m_w = tcom.real();//roundf(tcom.real()*1000)/1000;
		m_angle = acos(m_w)* (360/M_PI);
		cout<<"\nangle = "<<m_angle<<endl;
		tcom = v(1);
		m_x = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"x-axis = "<<m_x / sin(m_angle * M_PI / 360)<<endl;
		tcom = v(2);
		m_y = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"y-axis = "<<m_y / sin(m_angle * M_PI / 360)<<endl;
		tcom = v(3);
		m_z = tcom.real();//roundf(tcom.real()*1000)/1000;
		cout<<"z-axis = "<<m_z / sin(m_angle * M_PI / 360)<<endl;
	}
}

Rect* Worker::getROI()
{
	return r;
}
