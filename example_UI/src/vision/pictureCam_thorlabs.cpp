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
#include <uc480.h>
#include "uc480class.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

//string filename = "out_camera_data.yml";

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
	//TODO: these 2 should be read from "out_camera_data.yml"
	distortion = (Mat_<double>(5,1) << 7.2914041963856420e+000, 2.0979596618049214e+000, -2.3690347009888141e-001, 5.1098820884635382e-002, 1.4682521220954941e-003 );
	intrinsics =  (Mat_<double>(3,3) << 2.0529114902590052e+004, 0., 6.3928941571612074e+002, 0.,
			2.5657205522615815e+004, 5.1836001554552104e+002, 0., 0., 1.) ;
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

Mat pictureCam_thorlabs::calculateRotationMatrix(Rect r)
{
	//TODO: these values should not be hardcoded
	int boardHeight = 3;
	int boardWidth = 4;
	Size cbSize = Size(boardHeight,boardWidth);

	vector<Point2d> imagePoints;
	bool found = false;
	Mat img = Mat(1024, 1280, CV_8UC3, getPcImageMemory());
	cvtColor(img, img, COLOR_BGR2GRAY);
//	resize(img, img, Size(0, 0), 0.5, 0s.5);
	Scalar w = Scalar(255,255,255);
	Scalar o = Scalar(128,128,128);
	//detect chessboard corners
	int key = 0;
	while(!found)
	{
		render();

		img = Mat(1024, 1280, CV_8UC3, getPcImageMemory());
		img = img(r);	//get the sub-image we are looking at
		cvtColor(img, img, COLOR_BGR2GRAY);
		resize(img, img, Size(0, 0), 0.5, 0.5);
		found = findChessboardCorners(img, cbSize, imagePoints,
				CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
				+ CALIB_CB_FAST_CHECK);
		for (int i = 0; i < imagePoints.size(); i++)
		{
			if(i == boardHeight-1 || i == boardHeight*(boardWidth - 1))
				circle(img, imagePoints[i], 4 , o);
			else
				circle(img, imagePoints[i], 4 , w);
		}
		imshow("test", img);
		key = waitKey(1);
		if (key==27)
			break;
	}

	//	Mat intrinsics;
	Mat rvec = Mat(Size(3,1), CV_64F);
	Mat tvec = Mat(Size(3,1), CV_64F);

	//setup vectors to hold the chessboard corners in the chessboard coordinate system and in the image
	vector<Point3d> boardPoints, framePoints;

	//generate vectors for the points on the chessboard
	for (int i=0; i<boardWidth; i++)
	{
		for (int j=0; j<boardHeight; j++)
		{
			boardPoints.push_back( Point3d( double(i), double(j), 0.0) );
		}
	}

	//generate points in the reference frame
	framePoints.push_back( Point3d( 0.0, 0.0, 0.0 ) );
	framePoints.push_back( Point3d( 5.0, 0.0, 0.0 ) );
	framePoints.push_back( Point3d( 0.0, 5.0, 0.0 ) );
	framePoints.push_back( Point3d( 0.0, 0.0, 5.0 ) );

	cout<<"lets solve it"<<endl;
	//find the camera extrinsic parameters
	//If the distortion is NULL/empty, the zero distortion coefficients are assumed
	try {
		solvePnPRansac( Mat(boardPoints), Mat(imagePoints), intrinsics, distortion, rvec, tvec, false ,200,8.0,0.95);	//the last value describes how accurate it should be (value should be between 0 - 1)
	} catch (Exception e) {
		cout<<"error @:"<<e.what()<<endl;
	}
	//			//show the pose estimation data
	cout << fixed << setprecision(4) << "rvec = ["
			<< rvec.at<double>(0,0) << ", "
			<< rvec.at<double>(1,0) << ", "
			<< rvec.at<double>(2,0) << "] \t" << "tvec = ["
			<< tvec.at<double>(0,0) << ", "
			<< tvec.at<double>(1,0) << ", "
			<< tvec.at<double>(2,0) << "]" << endl;

//	cout << "rvec = " << rvec<< "\ttvec = "<< tvec<< endl;


	Mat rotationMatrix;

	Rodrigues(rvec,rotationMatrix);
	return rotationMatrix;
}

Mat pictureCam_thorlabs::calculateU(Mat a, Mat b)
{

	Mat t0 = (b-(a.dot(b))*a)/norm(b-(a.dot(b))*a);
	Mat t1 = a.cross(b);
	Mat f = (Mat_<double>(3,3) << a.at<double>(0,0) , t0.at<double>(0,0), t1.at<double>(0,0),
									a.at<double>(0,1) , t0.at<double>(0,1), t1.at<double>(0,1),
									a.at<double>(0,2) , t0.at<double>(0,2), t1.at<double>(0,2));
//	cout<<"\nf = "<<f<<endl;
	Mat f1 = f.inv();
//	cout<<"\nf1 = "<<f1<<endl;

//	cout<<"\nf1*f = (this should be the identity matrix) "<<(f1*f)<<endl;
	Mat g;
	g = (Mat_<double>(3,3) << a.dot(b), - norm(a.cross(b)), 0,	norm(a.cross(b)), a.dot(b), 0,	0,0,1);
	Mat u = f1.inv() * g * f1;
	cout<< "\nu = " << u <<endl;
	return u;
}
