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
#include <iomanip>
#include <math.h>

#include "pictureCam_thorlabs.hpp"
//#include <Eigen>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>
//#include "opencv2/core/eigen.hpp"
//#include <LU>

using namespace cv;
using namespace std;
//using namespace Eigen;

//probably should not define them here like this, but it makes it a lot easier for me
pictureCam_thorlabs *pct;
double badMeh, avg;
int ndx, u, rndx;
int l, r;
string filename = "out_camera_data.yml";
Mat intrinsics, distortion;
double avals[200] = {};
double bvals[200] = {};
double cvals[200] = {};
double dvals[200] = {};
Mat a, b, c, d, q;
Mat x1, x2;
Mat tvecOld1, tvecOld2;
Mat n;

/*
 * PictureCam_Thorlabs is basically the same as uc480Live from Thorlabs only without MFC
 */
pictureCam_thorlabs::pictureCam_thorlabs(HWND h)
{
	m_hDisplay = h;			// handle to diplay window
	m_Ret = 0;			// return value of SDK functions
	m_nColorMode = 0;	// Y8/RGB16/RGB24/RGB32
	m_nBitsPerPixel = 0;// number of bits needed store one pixel
	m_nSizeX = 0;		// width of video
	m_nSizeY = 0;		// height of video
	m_pcImageMemory = 0;// image memory - pointer to buffer
	m_lMemoryId = 0;    // image memory - buffer ID
	m_nDispModeSel = e_disp_mode_bitmap;
	m_cameraLoaded = OpenCamera();       // open a camera handle
}

pictureCam_thorlabs::~pictureCam_thorlabs()
{
	//	Disconnect();
}

///////////////////////////////////////////////////////////////////////////////
//
// METHOD Cuc480LiveDlg::InitDisplayMode()
//
// DESCRIPTION: - initializes the display mode
//
///////////////////////////////////////////////////////////////////////////////
int pictureCam_thorlabs::InitDisplayMode()
{
	if (!m_camera.IsInit())
	{
		return IS_NO_SUCCESS;
	}

	if (m_pcImageMemory != NULL)
	{
		m_camera.FreeImageMem(m_pcImageMemory, m_lMemoryId);
	}

	m_pcImageMemory = NULL;

	switch (m_nDispModeSel)
	{
	case e_disp_mode_direct3D:
		// if initializiing the direct draw mode succeeded then
		// leave the switch tree else run through to bitmap mode
		m_Ret = m_camera.SetDisplayMode(IS_SET_DM_DIRECT3D);
		m_Ret = m_camera.DirectRenderer(DR_ENABLE_SCALING, NULL, NULL);

		if(m_Ret == IS_SUCCESS )
		{
			// setup the color depth to the current VGA setting
			m_camera.GetColorDepth( &m_nBitsPerPixel, &m_nColorMode );
			break;
		}
		else
		{
			m_nDispModeSel = e_disp_mode_bitmap;
		}

	case e_disp_mode_bitmap:
		m_Ret = m_camera.SetDisplayMode(IS_SET_DM_DIB);
		if ((m_sInfo.nColorMode == IS_COLORMODE_BAYER) ||
				(m_sInfo.nColorMode == IS_COLORMODE_CBYCRY))
		{
			// for color camera models use RGB24 mode
			m_nColorMode = IS_SET_CM_RGB24;
			m_nBitsPerPixel = 24;
		}
		else
		{
			// for monochrome camera models use Y8 mode
			m_nColorMode = IS_SET_CM_Y8;
			m_nBitsPerPixel = 8;
		}
		// allocate an image memory.
		if (m_camera.AllocImageMem( m_nSizeX,
				m_nSizeY,
				m_nBitsPerPixel,
				&m_pcImageMemory,
				&m_lMemoryId ) != IS_SUCCESS )
		{
			cout<<"Memory allocation failed!"<<endl;
		}
		else
			m_camera.SetImageMem( m_pcImageMemory, m_lMemoryId );
		break;
	}   // end switch(m_nDispModeSel)

	if(m_Ret == IS_SUCCESS )
	{
		// set the desired color mode
		m_camera.SetColorMode( m_nColorMode );

		// set the image size to capture
		m_camera.SetImageSize( m_nSizeX, m_nSizeY );

	}   // end if(m_Ret == IS_SUCCESS )

	return m_Ret;
}

///////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: - Opens a handle to a connected camera
//
///////////////////////////////////////////////////////////////////////////////
bool pictureCam_thorlabs::OpenCamera()
{
	ExitCamera();

	// init camera
	m_Ret = m_camera.InitCamera(0, m_hDisplay );        // init cam

	// continue when camera is sucessfully initialized
	if( m_Ret == IS_SUCCESS )
	{
		// retrieve original image size
		m_camera.GetSensorInfo(&m_sInfo );

		m_camera.GetMaxImageSize(&m_nSizeX, &m_nSizeY);

		m_Ret = InitDisplayMode();

		// start live video
		m_camera.CaptureVideo(IS_WAIT);

		// do the whitebalance once on the first acquisitioned image only on color cameras
		if( m_sInfo.nColorMode == IS_COLORMODE_BAYER )
			m_camera.SetWhiteBalance(IS_SET_WB_AUTO_ENABLE);

		return true;
	}
	else
	{
		cout<<"No camera could be opened !"<<endl;
	}

	return false;

}

///////////////////////////////////////////////////////////////////////////////
//
// METHOD Cuc480LiveDlg::ExitCamera()
//
// DESCRIPTION: - exits the instance of the camera
//
///////////////////////////////////////////////////////////////////////////////
void pictureCam_thorlabs::ExitCamera()
{
	if( m_camera.IsInit() )
	{
		m_camera.EnableMessage( IS_FRAME, NULL );
		m_camera.StopLiveVideo( IS_WAIT );
		if( m_pcImageMemory != NULL )
			m_camera.FreeImageMem( m_pcImageMemory, m_lMemoryId );

		m_pcImageMemory = NULL;
		m_camera.ExitCamera();
	}     // end if( m_camera.IsInit() )
}


HWND pictureCam_thorlabs::getHDisplay() {
	return m_hDisplay;
}

void pictureCam_thorlabs::setHDisplay(HWND hDisplay) {
	m_hDisplay = hDisplay;
}

INT pictureCam_thorlabs::getNSizeX()  {
	return m_nSizeX;
}

void pictureCam_thorlabs::setNSizeX(INT nSizeX) {
	m_nSizeX = nSizeX;
}

INT pictureCam_thorlabs::getNSizeY()  {
	return m_nSizeY;
}

void pictureCam_thorlabs::setNSizeY(INT nSizeY) {
	m_nSizeY = nSizeY;
}

INT pictureCam_thorlabs::getRet()  {
	return m_Ret;
}

void pictureCam_thorlabs::setRet(INT ret) {
	m_Ret = ret;
}

char* pictureCam_thorlabs::getPcImageMemory()  {
	return m_pcImageMemory;
}

void pictureCam_thorlabs::setPcImageMemory(char* pcImageMemory) {
	m_pcImageMemory = pcImageMemory;
}

INT pictureCam_thorlabs::getLMemoryId()  {
	return m_lMemoryId;
}

void pictureCam_thorlabs::setLMemoryId(INT lMemoryId) {
	m_lMemoryId = lMemoryId;
}

Cuc480 pictureCam_thorlabs::getCamera()  {
	return m_camera;
}

HCAM  pictureCam_thorlabs::GetCameraHandle ()
{
	return m_camera.GetCameraHandle();
}

HWND  pictureCam_thorlabs::GetWindowHandle ()
{
	return m_camera.GetWindowHandle();
}

HWND  pictureCam_thorlabs::GetNotifyWindowHandle ()
{
	return m_camera.GetNotifyWindowHandle();
}

/*
 * different from m_camera.isInit()
 * this checks if a camera was opened
 */
BOOL  pictureCam_thorlabs::IsInit()
{
	return m_cameraLoaded;

}

/*
 * render data from camera to display
 */
void pictureCam_thorlabs::render()
{
	if(m_pcImageMemory != NULL)
		m_camera.RenderBitmap(m_lMemoryId, m_hDisplay, IS_RENDER_FIT_TO_WINDOW);
}

/*
 * with default fps, the exposure range is roughly between 0.07 - 99 ms
 */
void pictureCam_thorlabs::setExposure(int value)
{
	double e = 0;
	if(m_camera.IsInit())
	{
		cout<<"\n<----------- Begin of exposure ------------->"<<endl;
		double min = 10;
		double max = 25;
		double interval = 3;
		double tmp= 0;
		m_camera.is_Exposure(m_camera.GetCameraHandle(), IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MIN, &min, sizeof(min));
		m_camera.is_Exposure(m_camera.GetCameraHandle(), IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_MAX, &max, sizeof(max));
		m_camera.is_Exposure(m_camera.GetCameraHandle(), IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE_INC, &interval, sizeof(interval));
		cout << "the minimum exposure is "<< min << ", the maximum exposure is "<< max << ", with an interval of "<< interval << endl;
		if(value>0 && value<=20)
		{
			e = (max*value);
			e /= 20;
		}
		else if(value>20)
		{
			e = max;
		}
		else
		{
			e = min;
		}
		tmp = e;
		m_camera.is_Exposure(m_camera.GetCameraHandle(), IS_EXPOSURE_CMD_SET_EXPOSURE, &e, sizeof(e));
		cout<<"tried to set exposure to "<<tmp <<", actually set to " << e <<endl;

		min =  max =  interval = 0;

		m_camera.GetFpsRange(&min, &max, &interval);
		cout << "the minimum fps is "<< min << ", the maximum fps is "<< max << ", with an interval of "<< interval << endl;

		double fps = 0;
		m_camera.is_GetFramesPerSecond(m_camera.GetCameraHandle(),&fps);
		double exp=0;
		m_camera.is_Exposure(m_camera.GetCameraHandle(), IS_EXPOSURE_CMD_GET_EXPOSURE, &exp, sizeof(exp));
		cout<<"current FPS: " << fps<<", Current exposure: "<<exp<<endl;
		cout<<"<--------- End of exposure ------------->"<<endl;
	}
	else
	{
		cout << "something went wrong and there is no open camera."<< endl;
	}
}

void pictureCam_thorlabs::setFps(int value)
{
	if(m_camera.IsInit())
	{
		double fps = value+1;
		double min = 0, max = 0, interval = 0;

		m_camera.GetFpsRange(&min, &max, &interval);
		cout << "the minimum fps is "<< min << ", the maximum fps is "<< max << ", with an interval of "<< interval << endl;
		if(value>1 && value<=24)
		{
			fps *= (max/25);
			//			fps /= 24;
		}
		else if(value>24)
		{
			fps = max;
		}
		else
		{
			fps = min;
		}
		double tmp = fps;
		m_camera.SetFrameRate(fps, &fps);
		cout<<"tried to set fps to "<<tmp <<", actually  set to " << fps <<endl;
	}
	else
	{
		cout << "something went wrong and there is no open camera."<< endl;
	}
}

void pictureCam_thorlabs::setPixelClock(int value)
{
	if(m_camera.IsInit())
	{
		cout<<"\n<------------ Begin of pixel clock ------------->"<<endl;
		int i = 0;
		double min = 0, max = 0, interval = 0;
		m_camera.is_PixelClock(m_camera.GetCameraHandle(), IS_PIXELCLOCK_CMD_GET_DEFAULT, &i, sizeof(i));
		cout<<"the default pixelclock setting is "<<i<<endl;

		//		m_camera.is_SetOptimalCameraTiming(m_camera.GetCameraHandle(), IS_BEST_PCLK_RUN_ONCE, 4000, &i, &max);
		//		cout<<"the optimal settings for the pixel clock are "<<i<<", for the fps it is "<<max<<endl;

		double pc = value+5;
		m_camera.SetPixelClock(pc);
		cout<<"pixel clock was set to " << pc <<endl;

		m_camera.GetFpsRange(&min, &max, &interval);
		cout << "the minimum fps is "<< min << ", the maximum fps is "<< max << ", with an interval of "<< interval << endl;

		double fps = 0;
		m_camera.is_GetFramesPerSecond(m_camera.GetCameraHandle(),&fps);
		double exp=0;
		m_camera.is_Exposure(m_camera.GetCameraHandle(), IS_EXPOSURE_CMD_GET_EXPOSURE, &exp, sizeof(exp));
		cout<<"current FPS: " << fps<<", Current exposure: "<<exp<<endl;
		cout<<"<------------ End of pixel clock ------------->"<<endl;
	}
	else
	{
		cout << "something went wrong and there is no open camera."<< endl;
	}
}

/*
 * operations used by 'Main'
 */

static void on_expoTrackbar(int i, void* )
{
	pct->setExposure(i);
}

//static void on_fpsTrackbar(int i, void* )
//{
//	//	pictureCam_thorlabs *pct =(pictureCam_thorlabs*) p;
//	pct->setFps(i);
//}

static void on_pixelTrackbar(int i, void* )
{
	//	pictureCam_thorlabs *pct =(pictureCam_thorlabs*) p;
	pct->setPixelClock(i);
}

void calcHist(Mat *src)
{
	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat g_hist;

	/// Compute the histograms:
	calcHist( src, 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );

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
	imshow("histogram", histImage );
}

/*
 * returns the angle (in degrees) between 2 points and the origin
 */
float angleBetween(const Point &v1, const Point &v2)
{
	float len1 = sqrt(v1.x * v1.x + v1.y * v1.y);
	float len2 = sqrt(v2.x * v2.x + v2.y * v2.y);

	float dot = v1.x * v2.x + v1.y * v2.y;

	float a = dot / (len1 * len2);

	return acos(a) * (180.0 / M_PI); // 0..PI
}

/*
 *  I got this example for pose estimation from:
 *  https://www.youtube.com/watch?v=hUlIkHCQKmY
 *  code @: https://github.com/foxymop/3DPoseEstimation/blob/master/src/coordinate_system.cpp
 *  In the sample code this was done in the main, so it needs to be reworked a bit
 *
 *  A --- B
 *  |     |
 *  D --- C
 */
int getChessOrientation(Mat img)
{
	//TODO: update values
	int boardHeight = 4;
	int boardWidth = 6;
	Size cbSize = Size(boardHeight,boardWidth);

	vector<Point2d> imagePoints;
	bool found = false;

	//detect chessboard corners
	try {
		found = findChessboardCorners(img, cbSize, imagePoints,
				CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
				+ CALIB_CB_FAST_CHECK);
	} catch (Exception e) {
		cout<<"we had an error here"<<e.what()<<endl;
	}

	CvScalar w = CV_RGB(255,255,255);
	CvScalar o = CV_RGB(128,128,128);

	for (int i = 0; i < imagePoints.size(); i++)
	{
		if(i == boardHeight-1 || i == boardHeight*(boardWidth - 1))
			circle(img, imagePoints[i], 4 , o);
		else
			circle(img, imagePoints[i], 4 , w);
	}

	//	drawChessboardCorners(img, cbSize, Mat(imagePoints), found);
	//find camera orientation if the chessboard corners have been found
	if ( found )
	{
		//		cout<<"\n<----------- Estimated pose -------------->"<<endl;
		Mat rvec = Mat(Size(3,1), CV_64F);
		Mat tvec = Mat(Size(3,1), CV_64F);
		Mat tvecO = Mat(Size(3,1), CV_64F);

		//setup vectors to hold the chessboard corners in the chessboard coordinate system and in the image
		vector<Point3d> boardPoints;

		//generate vectors for the points on the chessboard
		for (int i=0; i<boardWidth; i++)
		{
			for (int j=0; j<boardHeight; j++)
			{
				boardPoints.push_back( Point3d( double(i), double(j), 0.0) );
			}
		}

		//calculate the angle between the last point in the first row and the last point in the first collum.
		//this should be 90 degrees, however since my camera is angled at 45 degrees and the platform can be angled too, I should keep this into account
		Point2d p1, p2;
		p1.x = imagePoints[boardHeight-1].x - imagePoints[0].x;
		p1.y = imagePoints[boardHeight-1].y - imagePoints[0].y;
		p2.x = imagePoints[boardHeight*(boardWidth - 1)].x - imagePoints[0].x;
		p2.y = imagePoints[boardHeight*(boardWidth - 1)].y - imagePoints[0].y;
		//		double a = angleBetween(p1,p2);
		//		cout<<"angle is "<<a<<endl;
		//		if(a>70 && a<110)
		//		{
		vector<Point2d> imgPoints;
		//			//if-statement magic ....	//TODO: change to something that works in any situation (like the corner distance thing)
		if(p1.x<20 && p1.x>-20)
		{
			if(p2.x>0 && p1.y<0)	//A,
			{
				cout<<"\nD1"<<endl;
				//				dvals[l] = a;
				//				l++;
				imgPoints = imagePoints;
				for(int i = 1; i<=boardHeight*boardWidth; i++)
				{
					imagePoints[i-1] = imgPoints[(boardHeight*boardWidth)-i];
				}
				//				Mat temp = tvecOld2;
				//				tvecOld2 = tvecOld1;
				//				tvecOld1 = temp;
			}
			else if(p2.x>0 && p1.y>0) //D,
			{
				cout<<"A1"<<endl;
				//				avals[u] = a;
				//				u++;
			}
			else if(p2.x<0 && p1.y<0)	//C,
			{
				cout<<"C1"<<endl;
				//				cvals[d] = a;
				//				d++;
			}
			else if(p2.x<0 && p1.y>0) //B,
			{
				cout<<"\nB1->D1"<<endl;
				//				bvals[r] = a;
				//				r++;
				imgPoints = imagePoints;
				for(int i = 1; i<=boardHeight*boardWidth; i++)
				{
					imgPoints[i-1] = imagePoints[(boardHeight*boardWidth)-i];
				}
			}
		}
		else	//'long arm' horizontal
		{
			if(p1.x>0 && p2.y<0)	//A, 'long arm' right, 'short arm' down
			{
				cout<<"A2"<<endl;
				//				avals[u] = a;
				//				u++;
			}
			else if(p1.x>0 && p2.y>0) //D, 'long arm' right, 'short arm' up
			{
				cout<<"D2"<<endl;

				//				dvals[l] = a;
				//				l++;
			}
			else if(p1.x<0 && p2.y<0)	//C, 'long arm' left, 'short arm' up
			{
				cout<<"C2"<<endl;
				//				cvals[d] = a;
				//				d++;
			}
			else if(p1.x<0 && p2.y>0) //B, 'long arm' left, 'short arm' down
			{
				cout<<"B2"<<endl;
				//				bvals[r] = a;
				//				r++;
			}
		}

		//find the camera extrinsic parameters
		//If the distortion is NULL/empty, the zero distortion coefficients are assumed
		line(img, imgPoints[0],imgPoints[boardHeight-1],w);
		line(img, imgPoints[0],imgPoints[boardHeight*(boardWidth - 1)],w);
		solvePnPRansac( Mat(boardPoints), Mat(imgPoints), intrinsics, distortion, rvec, tvec, false ,200,8.0,200);

		solvePnPRansac( Mat(boardPoints), Mat(imagePoints), intrinsics, distortion, rvec, tvecO, false ,200,8.0,200);

		if(ndx==0)
		{
			tvecOld1 = tvec;
			tvecOld2 = tvecO;
			avals[ndx] = roundf(tvec.at<double>(0,0)*100)/100 ;//- tvecOld1.at<double>(0,0);
			bvals[ndx] = roundf(tvec.at<double>(1,0)*100)/100 ;//- tvecOld1.at<double>(1,0);
			cvals[ndx] = roundf(tvec.at<double>(2,0)) ; ;//- tvecOld2.at<double>(2,0);

		}
		else
		{
			cout<<"\nindex = "<<ndx<<endl;
			//			double d1 = pow(pow(tvec.at<double>(0,0) - tvecOld1.at<double>(0,0),2) + pow(tvec.at<double>(1,0) - tvecOld1.at<double>(1,0),2) + pow(tvec.at<double>(2,0) - tvecOld1.at<double>(2,0),2), 0.5);
			//			d1 = roundf(d1 * 100) / 100;	//round to 2 decimals
			////			cout<<"tvec , tvecOld1 = " << d1 << endl;
			//			double d2 = pow(pow(tvec.at<double>(0,0) - tvecOld2.at<double>(0,0),2) + pow(tvec.at<double>(1,0) - tvecOld2.at<double>(1,0),2) + pow(tvec.at<double>(2,0) - tvecOld2.at<double>(2,0),2), 0.5);
			//			d2 = roundf(d2 * 100) / 100;	//round to 2 decimals
			////			cout<<"tvec , tvecOld2 = " << d2 << endl;
			//
			//			double d3 = pow(pow(tvecO.at<double>(0,0) - tvecOld1.at<double>(0,0),2) + pow(tvecO.at<double>(1,0) - tvecOld1.at<double>(1,0),2) + pow(tvecO.at<double>(2,0) - tvecOld1.at<double>(2,0),2), 0.5);
			//			d3 = roundf(d3 * 100) / 100;	//round to 2 decimals
			////			cout<<"\ntvecO , tvecOld1 = " << d3 << endl;
			//			double d4 = pow(pow(tvecO.at<double>(0,0) - tvecOld2.at<double>(0,0),2) + pow(tvecO.at<double>(1,0) - tvecOld2.at<double>(1,0),2) + pow(tvecO.at<double>(2,0) - tvecOld2.at<double>(2,0),2), 0.5);
			//			d4 = roundf(d4 * 100) / 100;	//round to 2 decimals
			//			cout<<"tvecO , tvecOld2 = " << d4 << endl;

			//			if(d4>d3 && d1>2)	//a different origin was taken when compared to the last measurement
			//			{
			//				avals[ndx] = tvec.at<double>(0,0) ;//- tvecOld2.at<double>(0,0);
			//				bvals[ndx] = tvec.at<double>(1,0) ;//- tvecOld2.at<double>(1,0);
			//				cvals[ndx] = tvec.at<double>(2,0) ;//- tvecOld2.at<double>(2,0);
			//
			//				tvecOld1 = tvecO;
			//				tvecOld2 = tvec;
			//			}
			//			else	//same origin
			//			{
			avals[ndx] = roundf(tvec.at<double>(0,0)*100)/100 ;//- tvecOld1.at<double>(0,0);
			bvals[ndx] = roundf(tvec.at<double>(1,0)*100)/100 ;//- tvecOld1.at<double>(1,0);
			cvals[ndx] = roundf(tvec.at<double>(2,0)) ;//- tvecOld1.at<double>(2,0);
			cout<<"tvec = "<<tvec<<endl;
			if(ndx%5 == 0)
			{
				int i = floor(ndx/5)-1;
				dvals[i] = pow(pow(avals[ndx] - avals[ndx-1],2) + pow(bvals[ndx] - bvals[ndx-1],2) + pow(cvals[ndx] - cvals[ndx-1],2),0.5);
			}
			cout<<"distance = " << pow(pow(avals[ndx] - avals[ndx-1],2) + pow(bvals[ndx] - bvals[ndx-1],2) + pow(cvals[ndx] - cvals[ndx-1],2),0.5)  <<endl;
			tvecOld1 = tvec;
			tvecOld2 = tvecO;
			//			}
		}
		ndx++;
		return 1;
	}
	else
	{
		//		cout<<"failed to detect corners"<<endl;
		return 0;
	}
}

// Given two sets of 3D points, find the rotation + translation + scale
// which best maps the first set to the second.
// Source: http://en.wikipedia.org/wiki/Kabsch_algorithm

// The input 3D points are stored as columns.
//Eigen::Affine3d Find3DAffineTransform(Eigen::Matrix3Xd in, Eigen::Matrix3Xd out) {
//
//	// Default output
//	Eigen::Affine3d A;
//	A.linear() = Eigen::Matrix3d::Identity(3, 3);
//	A.translation() = Eigen::Vector3d::Zero();
//
//	if (in.cols() != out.cols())
//		throw "Find3DAffineTransform(): input data mis-match";
//
//	// First find the scale, by finding the ratio of sums of some distances,
//	// then bring the datasets to the same scale.
//	double dist_in = 0, dist_out = 0;
//	Eigen::Vector3d v;
//	cout<<"in = "<<in<<endl;
//	for (int col = 0; col < in.cols()-1; col++) {
//		v = (in.col(col+1) - in.col(col));
//		cout<<"v = "<<v<<endl;
//		dist_in  += v.norm();
//		v = (out.col(col+1) - out.col(col));
//		dist_out += v.norm();
//	}
//	if (dist_in <= 0 || dist_out <= 0)
//		return A;
//	double scale = dist_out/dist_in;
//	out /= scale;
//
//	// Find the centroids then shift to the origin
//	Eigen::Vector3d in_ctr = Eigen::Vector3d::Zero();
//	Eigen::Vector3d out_ctr = Eigen::Vector3d::Zero();
//	for (int col = 0; col < in.cols(); col++) {
//		in_ctr  += in.col(col);
//		out_ctr += out.col(col);
//	}
//	in_ctr /= in.cols();
//	out_ctr /= out.cols();
//	for (int col = 0; col < in.cols(); col++) {
//		in.col(col)  -= in_ctr;
//		out.col(col) -= out_ctr;
//	}
//
//	// SVD
//	Eigen::MatrixXd Cov = in * out.transpose();
//	Eigen::JacobiSVD<Eigen::MatrixXd> svd(Cov, Eigen::ComputeThinU | Eigen::ComputeThinV);
//
//	// Find the rotation
//	MatrixXd temp = (svd.matrixV() * svd.matrixU().transpose());
//	double d = temp.determinant();
//	if (d > 0)
//		d = 1.0;
//	else
//		d = -1.0;
//	Eigen::Matrix3d I = Eigen::Matrix3d::Identity(3, 3);
//	I(2, 2) = d;
//	Eigen::Matrix3d R = svd.matrixV() * I * svd.matrixU().transpose();
//
//	// The final transform
//	A.linear() = scale * R;
//	A.translation() = scale*(out_ctr - R*in_ctr);
//
//	return A;
//}

int roots(double *a,int n,double *wr,double *wi)
{
    double sq,b2,c,disc;
    int m, numroots;

    m = n;
    numroots = 0;
    while (m > 1) {
        b2 = -0.5*a[m-2];
        c = a[m-1];
        disc = b2*b2-c;
        if (disc < 0.0) {                   // complex roots
            sq = sqrt(-disc);
            wr[m-2] = b2;
            wi[m-2] = sq;
            wr[m-1] = b2;
            wi[m-1] = -sq;
            numroots+=2;
        }
        else {                              // real roots
            sq = sqrt(disc);
            wr[m-2] = fabs(b2)+sq;
            if (b2 < 0.0) wr[m-2] = -wr[m-2];
            if (wr[m-2] == 0)
                wr[m-1] = 0;
            else {
                wr[m-1] = c/wr[m-2];
                numroots+=2;
            }
            wi[m-2] = 0.0;
            wi[m-1] = 0.0;
        }
        m -= 2;
    }
    if (m == 1) {
       wr[0] = -a[0];
       wi[0] = 0.0;
       numroots++;
    }
    return numroots;
}
//
// Deflate polynomial 'a' by dividing out 'quad'. Return quotient
// polynomial in 'b' and error metric based on remainder in 'err'.
//
void deflate(double *a,int n,double *b,double *quad,double *err)
{
    double r,s;
    int i;

    r = quad[1];
    s = quad[0];

    b[1] = a[1] - r;

    for (i=2;i<=n;i++){
        b[i] = a[i] - r * b[i-1] - s * b[i-2];
    }
    *err = fabs(b[n])+fabs(b[n-1]);
}
//
// Find quadratic factor using Bairstow's method (quadratic Newton method).
// A number of ad hoc safeguards are incorporated to prevent stalls due
// to common difficulties, such as zero slope at iteration point, and
// convergence problems.
//
// Bairstow's method is sensitive to the starting estimate. It is possible
// for convergence to fail or for 'wild' values to trigger an overflow.
//
// It is advisable to institute traps for these problems. (To do!)
//
void find_quad(double *a,int n,double *b,double *quad,double *err, int *iter)
{
    double *c,dn,dr,ds,drn,dsn,eps,r,s;
    int i;

    c = new double [n+1];
    c[0] = 1.0;
    r = quad[1];
    s = quad[0];
    eps = 1e-15;
    *iter = 1;

    do {
        if (*iter > 500) break;
        if (((*iter) % 200) == 0) {
            eps *= 10.0;
		}
		b[1] = a[1] - r;
		c[1] = b[1] - r;

		for (i=2;i<=n;i++){
			b[i] = a[i] - r * b[i-1] - s * b[i-2];
			c[i] = b[i] - r * c[i-1] - s * c[i-2];
		}
		dn=c[n-1] * c[n-3] - c[n-2] * c[n-2];
		drn=b[n] * c[n-3] - b[n-1] * c[n-2];
		dsn=b[n-1] * c[n-1] - b[n] * c[n-2];

        if (fabs(dn) < 1e-10) {
            if (dn < 0.0) dn = -1e-8;
            else dn = 1e-8;
        }
        dr = drn / dn;
        ds = dsn / dn;
		r += dr;
		s += ds;
        (*iter)++;
    } while ((fabs(dr)+fabs(ds)) > eps);
    quad[0] = s;
    quad[1] = r;
    *err = fabs(ds)+fabs(dr);
    delete [] c;
}
//
// Differentiate polynomial 'a' returning result in 'b'.
//
void diff_poly(double *a,int n,double *b)
{
    double coef;
    int i;

    coef = (double)n;
    b[0] = 1.0;
    for (i=1;i<n;i++) {
        b[i] = a[i]*((double)(n-i))/coef;
    }
}
//
// Attempt to find a reliable estimate of a quadratic factor using modified
// Bairstow's method with provisions for 'digging out' factors associated
// with multiple roots.
//
// This resursive routine operates on the principal that differentiation of
// a polynomial reduces the order of all multiple roots by one, and has no
// other roots in common with it. If a root of the differentiated polynomial
// is a root of the original polynomial, there must be multiple roots at
// that location. The differentiated polynomial, however, has lower order
// and is easier to solve.
//
// When the original polynomial exhibits convergence problems in the
// neighborhood of some potential root, a best guess is obtained and tried
// on the differentiated polynomial. The new best guess is applied
// recursively on continually differentiated polynomials until failure
// occurs. At this point, the previous polynomial is accepted as that with
// the least number of roots at this location, and its estimate is
// accepted as the root.
//
void recurse(double *a,int n,double *b,int m,double *quad,
    double *err,int *iter)
{
    double *c,*x,rs[2],tst;

    if (fabs(b[m]) < 1e-16) m--;    // this bypasses roots at zero
    if (m == 2) {
        quad[0] = b[2];
        quad[1] = b[1];
        *err = 0;
        *iter = 0;
        return;
    }
    c = new double [m+1];
    x = new double [n+1];
    c[0] = x[0] = 1.0;
    rs[0] = quad[0];
    rs[1] = quad[1];
    *iter = 0;
    find_quad(b,m,c,rs,err,iter);
    tst = fabs(rs[0]-quad[0])+fabs(rs[1]-quad[1]);
    if (*err < 1e-12) {
        quad[0] = rs[0];
        quad[1] = rs[1];
    }
// tst will be 'large' if we converge to wrong root
    if (((*iter > 5) && (tst < 1e-4)) || ((*iter > 20) && (tst < 1e-1))) {
        diff_poly(b,m,c);
        recurse(a,n,c,m-1,rs,err,iter);
        quad[0] = rs[0];
        quad[1] = rs[1];
    }
    delete [] x;
    delete [] c;
}
//
// Top level routine to manage the determination of all roots of the given
// polynomial 'a', returning the quadratic factors (and possibly one linear
// factor) in 'x'.
//
void get_quads(double *a,int n,double *quad,double *x)
{
    double *b,*z,err,tmp;
    int iter,i,m;

    if ((tmp = a[0]) != 1.0) {
        a[0] = 1.0;
        for (i=1;i<=n;i++) {
            a[i] /= tmp;
        }
    }
    if (n == 2) {
        x[0] = a[1];
        x[1] = a[2];
        return;
    }
    else if (n == 1) {
        x[0] = a[1];
        return;
    }
    m = n;
    b = new double [n+1];
    z = new double [n+1];
    b[0] = 1.0;
    for (i=0;i<=n;i++) {
        z[i] = a[i];
        x[i] = 0.0;
    }
    do {
        if (n > m) {
            quad[0] = 3.14159e-1;
            quad[1] = 2.78127e-1;
        }
        do {                    // This loop tries to assure convergence
            for (i=0;i<5;i++) {
                find_quad(z,m,b,quad,&err,&iter);
                if ((err > 1e-7) || (iter > 500)) {
                    diff_poly(z,m,b);
                    recurse(z,m,b,m-1,quad,&err,&iter);
                }
                deflate(z,m,b,quad,&err);
                if (err < 0.001) break;
                quad[0] = rand() % 8 - 4.0;
                quad[1] = rand() % 8 - 4.0;
            }
            if (err > 0.01) {
                cout << "Error! Convergence failure in quadratic x^2 + r*x + s." << endl;
                cout << "Enter new trial value for 'r': ";
                cin >> quad[1];
                cout << "Enter new trial value for 's' ( 0 to exit): ";
                cin >> quad[0];
                if (quad[0] == 0) exit(1);
            }
        } while (err > 0.01);
        x[m-2] = quad[1];
        x[m-1] = quad[0];
        m -= 2;
        for (i=0;i<=m;i++) {
            z[i] = b[i];
        }
    } while (m > 2);
    if (m == 2) {
        x[0] = b[1];
        x[1] = b[2];
    }
    else x[0] = b[1];
    delete [] z;
    delete [] b;
}

/*
 * @param a: 4x4 matrix
 * @param b: 4x4 matrix
 *
 * repeat this an x number of times
*/
void determineRotation(Mat a, Mat b)
{
	Mat a1 = Mat::zeros(4,4,CV_64F);
	Mat b1 = Mat::zeros(4,4,CV_64F);

	Mat a2 = Mat::zeros(4,4,CV_64F);
	Mat b2 = Mat::zeros(4,4,CV_64F);

	Mat a3 = Mat::zeros(4,4,CV_64F);
	Mat b3 = Mat::zeros(4,4,CV_64F);

	//Setting up data
	//first set
	a1.at<double>(0,1) = -(a.at<double>(0,0));
	a1.at<double>(1,0) = a.at<double>(0,0);
	a1.at<double>(0,2) = -(a.at<double>(1,0));
	a1.at<double>(2,0) = a.at<double>(1,0);
	a1.at<double>(0,3) = -(a.at<double>(2,0));
	a1.at<double>(3,0) = a.at<double>(2,0);
	a1.at<double>(1,2) = a.at<double>(2,0);
	a1.at<double>(2,1) = -(a.at<double>(2,0));
	a1.at<double>(1,3) = -(a.at<double>(1,0));
	a1.at<double>(3,1) = a.at<double>(1,0);
	a1.at<double>(2,3) = -(a.at<double>(0,0));
	a1.at<double>(3,2) = a.at<double>(0,0);

	b1.at<double>(0,1) = -(b.at<double>(0,0));
	b1.at<double>(1,0) = b.at<double>(0,0);
	b1.at<double>(0,2) = -(b.at<double>(1,0));
	b1.at<double>(2,0) = b.at<double>(1,0);
	b1.at<double>(0,3) = -(b.at<double>(2,0));
	b1.at<double>(3,0) = b.at<double>(2,0);
	b1.at<double>(1,2) = b.at<double>(2,0);
	b1.at<double>(2,1) = -(b.at<double>(2,0));
	b1.at<double>(1,3) = -(b.at<double>(1,0));
	b1.at<double>(3,1) = b.at<double>(1,0);
	b1.at<double>(2,3) = -(b.at<double>(0,0));
	b1.at<double>(3,2) = b.at<double>(0,0);

	//second set
	a2.at<double>(0,1) = -(a.at<double>(0,1));
	a2.at<double>(1,0) = a.at<double>(0,1);
	a2.at<double>(0,2) = -(a.at<double>(1,1));
	a2.at<double>(2,0) = a.at<double>(1,1);
	a2.at<double>(0,3) = -(a.at<double>(2,1));
	a2.at<double>(3,0) = a.at<double>(2,1);
	a2.at<double>(1,2) = a.at<double>(2,1);
	a2.at<double>(2,1) = -(a.at<double>(2,1));
	a2.at<double>(1,3) = -(a.at<double>(1,1));
	a2.at<double>(3,1) = a.at<double>(1,1);
	a2.at<double>(2,3) = -(a.at<double>(0,1));
	a2.at<double>(3,2) = a.at<double>(0,1);

	b2.at<double>(0,1) = -(b.at<double>(0,1));
	b2.at<double>(1,0) = b.at<double>(0,1);
	b2.at<double>(0,2) = -(b.at<double>(1,1));
	b2.at<double>(2,0) = b.at<double>(1,1);
	b2.at<double>(0,3) = -(b.at<double>(2,1));
	b2.at<double>(3,0) = b.at<double>(2,1);
	b2.at<double>(1,2) = b.at<double>(2,1);
	b2.at<double>(2,1) = -(b.at<double>(2,1));
	b2.at<double>(1,3) = -(b.at<double>(1,1));
	b2.at<double>(3,1) = b.at<double>(1,1);
	b2.at<double>(2,3) = -(b.at<double>(0,1));
	b2.at<double>(3,2) = b.at<double>(0,1);

	//third set
	a3.at<double>(0,1) = -(a.at<double>(0,2));
	a3.at<double>(1,0) = a.at<double>(0,2);
	a3.at<double>(0,2) = -(a.at<double>(1,2));
	a3.at<double>(2,0) = a.at<double>(1,2);
	a3.at<double>(0,3) = -(a.at<double>(2,2));
	a3.at<double>(3,0) = a.at<double>(2,2);
	a3.at<double>(1,2) = a.at<double>(2,2);
	a3.at<double>(2,1) = -(a.at<double>(2,2));
	a3.at<double>(1,3) = -(a.at<double>(1,2));
	a3.at<double>(3,1) = a.at<double>(1,2);
	a3.at<double>(2,3) = -(a.at<double>(0,2));
	a3.at<double>(3,2) = a.at<double>(0,2);

	b3.at<double>(0,1) = -(b.at<double>(0,2));
	b3.at<double>(1,0) = b.at<double>(0,2);
	b3.at<double>(0,2) = -(b.at<double>(1,2));
	b3.at<double>(2,0) = b.at<double>(1,2);
	b3.at<double>(0,3) = -(b.at<double>(2,2));
	b3.at<double>(3,0) = b.at<double>(2,2);
	b3.at<double>(1,2) = b.at<double>(2,2);
	b3.at<double>(2,1) = -(b.at<double>(2,2));
	b3.at<double>(1,3) = -(b.at<double>(1,2));
	b3.at<double>(3,1) = b.at<double>(1,2);
	b3.at<double>(2,3) = -(b.at<double>(0,2));
	b3.at<double>(3,2) = b.at<double>(0,2);

	n = (a1.t() * b1) + (a2.t() * b2) + (a3.t() * b3);

	//use laplace on a 4x4 matrix to get the zmax value (a*x^4 + b*x^3 + c*x^2 + d*x + e = 0)
		double a_val = 1;
		double b_val = -n.at<double>(0,0) - n.at<double>(1,1) - n.at<double>(2,2) - n.at<double>(3,3);

		// af + ak + ap + fk + fp + kp - lo - gj - nd - eb - ci - dm
		double c_val =  n.at<double>(0,0) * n.at<double>(1,1) +  n.at<double>(0,0) * n.at<double>(2,2) + n.at<double>(0,0) * n.at<double>(3,3)
				+ n.at<double>(1,1) * n.at<double>(2,2) + n.at<double>(1,1) * n.at<double>(3,3) + n.at<double>(2,2) * n.at<double>(3,3)
				- n.at<double>(2,3) * n.at<double>(3,2) - n.at<double>(3,1) * n.at<double>(1,3) - n.at<double>(0,1) * n.at<double>(1,0)
				- n.at<double>(0,2) * n.at<double>(2,0) - n.at<double>(0,3) * n.at<double>(3,0) - n.at<double>(2,1) * n.at<double>(1,2);

		//- afk - afp - akp + alo + agj + anh + bek + bep - bgi - bhm - cej + cip + cif - clm + dfm - dio + dkm - ekn - fkp + flo + jgp - jho - gln + nhk
		double d_val = -(n.at<double>(0,0) * n.at<double>(1,1) * n.at<double>(2,2))	//afk
			 - (n.at<double>(0,0) * n.at<double>(1,1) * n.at<double>(3,3))	//afp
			 - (n.at<double>(0,0) * n.at<double>(3,3) * n.at<double>(2,2))	//akp
			 + (n.at<double>(0,0) * n.at<double>(2,3) * n.at<double>(3,2))	//alo
			 + (n.at<double>(0,0) * n.at<double>(1,2) * n.at<double>(2,1))	//agj
			 + (n.at<double>(0,0) * n.at<double>(1,3) * n.at<double>(3,1))	//ahn
			 + (n.at<double>(0,1) * n.at<double>(1,0) * n.at<double>(2,2))	//bek
			 + (n.at<double>(0,1) * n.at<double>(1,0) * n.at<double>(3,3))	//bep
			 - (n.at<double>(0,1) * n.at<double>(1,3) * n.at<double>(3,0))	//bhm
			 - (n.at<double>(0,1) * n.at<double>(1,2) * n.at<double>(2,0))	//bgi
			 - (n.at<double>(0,2) * n.at<double>(1,0) * n.at<double>(2,1))	//cej
			 + (n.at<double>(0,2) * n.at<double>(1,1) * n.at<double>(2,0))	//cfi
			 + (n.at<double>(0,2) * n.at<double>(2,0) * n.at<double>(3,3))	//cip
			 - (n.at<double>(0,2) * n.at<double>(2,3) * n.at<double>(3,0))	//clm
			 + (n.at<double>(0,3) * n.at<double>(1,1) * n.at<double>(3,0))	//dfm
			 - (n.at<double>(0,3) * n.at<double>(2,0) * n.at<double>(3,2))	//dio
			 + (n.at<double>(0,3) * n.at<double>(2,2) * n.at<double>(3,0))	//dkm
			 - (n.at<double>(1,0) * n.at<double>(2,2) * n.at<double>(3,1))	//ekn
			 - (n.at<double>(1,1) * n.at<double>(2,2) * n.at<double>(3,3))	//fkp
			 + (n.at<double>(1,1) * n.at<double>(2,3) * n.at<double>(3,2))	//flo
			 - (n.at<double>(1,2) * n.at<double>(2,3) * n.at<double>(3,1))	//gln
			 + (n.at<double>(1,2) * n.at<double>(2,1) * n.at<double>(3,3))	//gjp
			 - (n.at<double>(1,3) * n.at<double>(2,1) * n.at<double>(3,2))	//hjo
			 + (n.at<double>(1,3) * n.at<double>(2,2) * n.at<double>(3,1));	//hkn

		//afkp - aflo - agjp + ahjo + agln - ahkn - bekp + belo - bglm + bgip - bhio + bhkm + cejp - celn - cfip + cflm + chin - chjm + dekn - dejo + dfio - dfkm - dgin + dgjm
		double e_val =  (n.at<double>(0,0) * n.at<double>(1,1) * n.at<double>(2,2) * n.at<double>(3,3))	//afkp
			 - (n.at<double>(0,0) * n.at<double>(1,1) * n.at<double>(2,3) * n.at<double>(3,2))	//aflo
			 - (n.at<double>(0,0) * n.at<double>(1,2) * n.at<double>(2,1) * n.at<double>(3,3))	//agjp
			 + (n.at<double>(0,0) * n.at<double>(1,3) * n.at<double>(2,1) * n.at<double>(3,2))	//ahjo
			 + (n.at<double>(0,0) * n.at<double>(1,2) * n.at<double>(2,3) * n.at<double>(3,1))	//agln
			 - (n.at<double>(0,0) * n.at<double>(1,3) * n.at<double>(2,2) * n.at<double>(3,1))	//ahkn
			 - (n.at<double>(0,1) * n.at<double>(1,0) * n.at<double>(2,2) * n.at<double>(3,3))	//bekp
			 + (n.at<double>(0,1) * n.at<double>(1,0) * n.at<double>(2,3) * n.at<double>(3,2))	//belo
			 + (n.at<double>(0,1) * n.at<double>(1,2) * n.at<double>(2,0) * n.at<double>(3,3))	//bgip
			 - (n.at<double>(0,1) * n.at<double>(1,2) * n.at<double>(2,3) * n.at<double>(3,0))	//bglm
			 - (n.at<double>(0,1) * n.at<double>(1,3) * n.at<double>(2,0) * n.at<double>(3,2))	//bhio
			 + (n.at<double>(0,1) * n.at<double>(1,3) * n.at<double>(2,2) * n.at<double>(3,0))	//bhkm
			 + (n.at<double>(0,2) * n.at<double>(1,0) * n.at<double>(2,1) * n.at<double>(3,3))	//cejp
			 - (n.at<double>(0,2) * n.at<double>(1,0) * n.at<double>(2,3) * n.at<double>(3,1))	//celn
			 - (n.at<double>(0,2) * n.at<double>(1,1) * n.at<double>(2,0) * n.at<double>(3,3))	//cfip
			 + (n.at<double>(0,2) * n.at<double>(1,1) * n.at<double>(2,3) * n.at<double>(3,0))	//cflm
			 + (n.at<double>(0,2) * n.at<double>(1,3) * n.at<double>(2,0) * n.at<double>(3,1))	//chin
			 - (n.at<double>(0,2) * n.at<double>(1,3) * n.at<double>(2,1) * n.at<double>(3,0))	//chjm
			 - (n.at<double>(0,3) * n.at<double>(1,0) * n.at<double>(2,1) * n.at<double>(3,2))	//dejo
			 + (n.at<double>(0,3) * n.at<double>(1,0) * n.at<double>(2,2) * n.at<double>(3,1))	//dekn
			 + (n.at<double>(0,3) * n.at<double>(1,1) * n.at<double>(2,0) * n.at<double>(3,2))	//dfio
			 - (n.at<double>(0,3) * n.at<double>(1,1) * n.at<double>(2,2) * n.at<double>(3,0))	//dfkm
			 - (n.at<double>(0,3) * n.at<double>(1,2) * n.at<double>(2,0) * n.at<double>(3,1))	//dgin
			 + (n.at<double>(0,3) * n.at<double>(1,2) * n.at<double>(2,1) * n.at<double>(3,0));	//dgjm

	//determinant = lapda^4 + (-ci - hn) * lapda^2 + (-dgjm +bglm +chin -belo)
	//	double dist =
	double vals[5],x[21],wr[21],wi[21],quad[2];	//wr holds the real values, while wi holds the imaginary values
	vals[0] = a_val;
	vals[1] = b_val;
	vals[2] = c_val;
	vals[3] = d_val;
	vals[4] = e_val;
	// initialize estimate for 1st root pair
	quad[0] = 2.71828e-1;
	quad[1] = 3.14159e-1;
	// get roots
	get_quads(vals,4,quad,x);
	int numr = roots(x,4,wr,wi);
	double zmax = wr[0];
	cout << endl << "Roots (" << numr << " found):" << endl;
	for (int i=0;i<4;i++) {
		if ((wr[i] != 0.0) || (wi[i] != 0.0)){
			cout << wr[i] << " " << wi[i] << "I" << endl;
			if(zmax<wr[i])
				zmax = wr[i];
		}
	}
	// (N -Zmax*I) v = 0
	cout<<"zmax = "<<zmax<<endl;
	Mat nzi = n - zmax * Mat::ones(4,4,CV_64F);
	Mat eigenVals, eigenVecs;
	eigen(nzi,eigenVals, eigenVecs);	//returns 4 vectors in row format, first 2 are the same as the last 2 but mirrored (negative)
	cout<<"eigen vectors:\n"<<eigenVecs<<endl;
	cout<<"eigen values:\n"<<eigenVals<<endl;
}

/*
 *	the first 4 calculate the center of rotation
 *	after that rotation is calculated
 */
int CalculateRotation(Mat img)
{
	//TODO: update values
	int boardHeight = 4;
	int boardWidth = 6;
	Size cbSize = Size(boardHeight,boardWidth);

	vector<Point2d> imagePoints;
	bool found = false;

	//detect chessboard corners
	try {
		found = findChessboardCorners(img, cbSize, imagePoints,
				CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
				+ CALIB_CB_FAST_CHECK);
	} catch (Exception e) {
		cout<<"we had an error here"<<e.what()<<endl;
	}

	CvScalar w = CV_RGB(255,255,255);
	CvScalar o = CV_RGB(128,128,128);

	for (int i = 0; i < imagePoints.size(); i++)
	{
		if(i == boardHeight-1 || i == boardHeight*(boardWidth - 1))
			circle(img, imagePoints[i], 4 , o);
		else
			circle(img, imagePoints[i], 4 , w);
	}

	//	drawChessboardCorners(img, cbSize, Mat(imagePoints), found);
	//find camera orientation if the chessboard corners have been found
	if ( found)
	{
		//		cout<<"\n<----------- Estimated pose -------------->"<<endl;
		Mat rvec = Mat(Size(3,1), CV_64F);
		Mat tvec = Mat(Size(3,1), CV_64F);

		//setup vectors to hold the chessboard corners in the chessboard coordinate system and in the image
		vector<Point3d> boardPoints;

		//generate vectors for the points on the chessboard
		for (int i=0; i<boardWidth; i++)
		{
			for (int j=0; j<boardHeight; j++)
			{
				boardPoints.push_back( Point3d( double(i), double(j), 0.0) );
			}
		}
		line(img, imagePoints[0],imagePoints[boardHeight-1],w);
		line(img, imagePoints[0],imagePoints[boardHeight*(boardWidth - 1)],w);
		solvePnPRansac( Mat(boardPoints), Mat(imagePoints), intrinsics, distortion, rvec, tvec, false ,200,8.0,200);

		cout<<"tvec = "<<tvec<<endl;
		Mat rmat;
		Rodrigues(rvec,rmat);
		cout<<"rotation matrix = "<<rmat<<endl;
		double a_abs, b_abs, c_abs, d_abs;
		Mat det, m;

		switch(rndx)
		{
		case 0:
			a = tvec;
			cout<<"a = "<< a <<endl;
			x1 = rmat;
			rndx++;
			break;
		case 1:
			b = tvec;
			cout<<"b = "<< b <<endl;
			rndx++;
			break;
		case 2:
			c = tvec;
			cout<<"c = "<< c <<endl;
			rndx++;
			break;
		case 3:
			d = tvec;
			cout<<"d = "<< d <<endl;
			x2 = rmat;
			rndx++;
			// |q|^2 -2qa - |a|^2 = |q|^2 -2qb - |b|^2
			// 2q(b-a) = |a|^2 - |b|^2
			// 2[ bx - ax, by - ay, bz - az] [qx;  = [|a|^2 - |b|^2 ;
			// 2[ cx - ax, cy - ay, cz - az]  qy;  =  |a|^2 - |c|^2 ;
			// 2[ dx - ax, dy - ay, dz - az]  qz]  =  |a|^2 - |d|^2 ]

			m = (Mat_<double>(3,3) << b.at<double>(0,0) - a.at<double>(0,0), b.at<double>(1,0) - a.at<double>(1,0), b.at<double>(2,0) - a.at<double>(2,0),
					c.at<double>(0,0) - a.at<double>(0,0), c.at<double>(1,0) - a.at<double>(1,0), c.at<double>(2,0) - a.at<double>(2,0),
					d.at<double>(0,0) - a.at<double>(0,0), d.at<double>(1,0) - a.at<double>(1,0), d.at<double>(2,0) - a.at<double>(2,0));

			a_abs = sqrt(pow(a.at<double>(0,0),2) + pow(a.at<double>(1,0),2) + pow(a.at<double>(2,0),2)) ;
			b_abs = sqrt(pow(b.at<double>(0,0),2) + pow(b.at<double>(1,0),2) + pow(b.at<double>(2,0),2));
			c_abs = sqrt(pow(c.at<double>(0,0),2) + pow(c.at<double>(1,0),2) + pow(c.at<double>(2,0),2));
			d_abs = sqrt(pow(d.at<double>(0,0),2) + pow(d.at<double>(1,0),2) + pow(d.at<double>(2,0),2));
			det = (Mat_<double>(3,1) <<	pow(a_abs,2) - pow(b_abs,2),
					pow(a_abs,2) - pow(c_abs,2),
					pow(a_abs,2) - pow(d_abs,2));
			cout << "det = "<<det <<endl;
			// 2 M * q = D
			// 2 q = M^-1 * D
			// q = 0.5 * M^-1 * D
			q = 0.5 * m.inv() * det;
			cout << "\nq = " << q <<endl;
			cout << "\n<--------------------------------------------------------->\n"<<endl;
			break;
		default:
			if(rndx >= 4){
			cout<<"\nprevious rotation:\n"<<x2<<endl;
			cout<<"current rotation:\n"<<rmat<<endl;
			determineRotation(rmat, x2);
			x2 = rmat;
			rndx++;
			}
			break;
		}
		return 1;
	}
	else
	{
		//		cout<<"failed to detect corners"<<endl;
		return 0;
	}
}

/*
 * this application is not optimized
 */
int main() {
	/*
	 * create the UI
	 */
	namedWindow( "Original", WINDOW_AUTOSIZE );
	resizeWindow("Original",1200,950);

	/*
	 * initialize the values we will be using
	 */
	pct = new pictureCam_thorlabs((HWND)cvGetWindowHandle("Original"));
	int pc = 25; 	// pixel clock values are between 5 and 40
	//	int fps = 24;	// max and min fps are based on the pixel clock
	int expo = 20;	// the exposure is depended of everything...
	Mat imgMat, element, grayImg;
	IplImage* cv_image ;
	bool running = false;
	ndx = rndx = u = l = r =  0;
	cv_image = cvCreateImageHeader(cvSize(1280,1024), IPL_DEPTH_8U, 3);

	/*
	 * create trackbars
	 */
	createTrackbar("Pixel clock", "Original", &pc, 35, on_pixelTrackbar);
	//	createTrackbar("fps_slider", "Original", &fps, 24, on_fpsTrackbar);
	createTrackbar("exposure_slider", "Original", &expo, 20, on_expoTrackbar);
	if( pct->IsInit() )
	{
		//set up matrices for storage
		distortion = Mat::zeros(5, 1, CV_64F);
		intrinsics = Mat::eye(3, 3, CV_64F);

		distortion = (Mat_<double>(5,1) << 7.1382655859702908e+001, -6.7509394728083327e-001,
			       3.5208124899178711e-001, -3.6677289190204306e-002,
			       -7.0415118500865041e-005 );
		intrinsics =  (Mat_<double>(3,3) << 5.7301009474144659e+004, 0., 6.3334059778234075e+002, 0.,
			       6.9675230432774188e+004, 5.3154756593783020e+002, 0., 0., 1. ) ;

		running = true;
	}
	bool printP = false;
	bool rotateP = false;

	tvecOld1 = Mat(Size(3,1), CV_64F);
	tvecOld2 = Mat(Size(3,1), CV_64F);
	//application loop
	while(running)
	{
		if( !pct->IsInit() )
			break;
		pct->render();
		//		cout<<"so slow ..."<<endl;
		/*
		 * Thorlabs does not support opencv so we need to do some of this ourselves
		 * convert image from buffer to an image we can actually use
		 */
		cvSetData(cv_image, pct->getPcImageMemory(), cv_image->widthStep);
		//! converts old-style IplImage to the new matrix; the data is not copied by default
		imgMat = Mat(cv_image, false);
		cvtColor(imgMat, grayImg, CV_BGR2GRAY);
		calcHist(&grayImg);

		if(printP)
			getChessOrientation(grayImg);
		if(rotateP)
			if(CalculateRotation(grayImg))
				rotateP = false;

		if(ndx % 5 == 0 && ndx != 0)
			printP = false;
		imshow("result", grayImg);

		switch(waitKey(1))
		{
		case 27:	//esc
			cout << "Esc key is pressed by user." << endl;
			cout << "Closing application." << endl;
			running = false;
			break;
		case 's':
			printP = !printP;
			cout<<"\nS pressed."<<endl;
			break;
		case 'p':
			rotateP = !rotateP;
			cout<<"\nP pressed."<<endl;
			break;
		default:
			break;
		}
		if(ndx>=200)
			running=false;
	}

	//	avg = 0;
	//	cout<<"\nThe difference in x is:\n"<<endl;
	//	for(int i = 0; i<ndx; i++)
	//	{
	//		cout << fixed <<setprecision(2)<< avals[i]<<"\t";
	//		avg += avals[i];
	//	}
	//	cout<<"\nThe average difference in x is "<<avg/(ndx-1)<<"\n"<<endl;
	//
	//	avg = 0;
	//	cout<<"\nThe difference in y is:\n"<<endl;
	//	for(int i = 0; i<ndx; i++)
	//	{
	//		cout << fixed <<setprecision(2)<< bvals[i]<<"\t";
	//		avg += bvals[i];
	//	}
	//	cout<<"\nThe average difference in y is "<<avg/(ndx-1)<<"\n"<<endl;
	//
	//	avg = 0;
	//	cout<<"\nThe difference in z is:\n"<<endl;
	//	for(int i = 0; i<ndx; i++)
	//	{
	//		cout << fixed <<setprecision(2)<< cvals[i]<<"\t";
	//		avg += cvals[i];
	//	}
	//	cout<<"\nThe average difference in z is "<<avg/(ndx-1)<<"\n"<<endl;

	avg = 0;
	cout<<"\nThe difference in x is:\n"<<endl;
	for(int i = 1; i-1<floor(ndx/5)-1; i++)
	{
		cout << fixed <<setprecision(4)<< dvals[i-1]<<"\t";
		if((i-1)%5 == 0)
			cout<<"\n";
		avg += dvals[i-1];
	}
	cout<<"\nThe average difference in x is "<<avg/(floor(ndx/5)-1)<<"\n"<<endl;
	pct->ExitCamera();
	pct = NULL;
	return 0;
}

