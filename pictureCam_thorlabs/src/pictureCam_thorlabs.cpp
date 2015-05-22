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
#include <complex>
#include <Eigen>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>
#include "opencv2/core/eigen.hpp"
//#include <LU>

using namespace cv;
using namespace std;
using namespace Eigen;

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
	a1.at<double>(0,1) = -(a.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0));
	a1.at<double>(1,0) = a.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0);
	a1.at<double>(0,2) = -(a.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0));
	a1.at<double>(2,0) = a.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0);
	a1.at<double>(0,3) = -(a.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0));
	a1.at<double>(3,0) = a.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0);
	a1.at<double>(1,2) = a.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0);
	a1.at<double>(2,1) = -(a.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0));
	a1.at<double>(1,3) = -(a.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0));
	a1.at<double>(3,1) = a.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0);
	a1.at<double>(2,3) = -(a.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0));
	a1.at<double>(3,2) = a.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0);

	b1.at<double>(0,1) = -(b.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0));
	b1.at<double>(1,0) = b.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0);
	b1.at<double>(0,2) = -(b.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0));
	b1.at<double>(2,0) = b.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0);
	b1.at<double>(0,3) = -(b.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0));
	b1.at<double>(3,0) = b.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0);
	b1.at<double>(1,2) = b.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0);
	b1.at<double>(2,1) = -(b.at<double>(2,0) + a.at<double>(2,3) - q.at<double>(2,0));
	b1.at<double>(1,3) = -(b.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0));
	b1.at<double>(3,1) = b.at<double>(1,0) + a.at<double>(1,3) - q.at<double>(1,0);
	b1.at<double>(2,3) = -(b.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0));
	b1.at<double>(3,2) = b.at<double>(0,0) + a.at<double>(0,3) - q.at<double>(0,0);

	//second set
	a2.at<double>(0,1) = -(a.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0));
	a2.at<double>(1,0) = a.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0);
	a2.at<double>(0,2) = -(a.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0));
	a2.at<double>(2,0) = a.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0);
	a2.at<double>(0,3) = -(a.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0));
	a2.at<double>(3,0) = a.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0);
	a2.at<double>(1,2) = a.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0);
	a2.at<double>(2,1) = -(a.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0));
	a2.at<double>(1,3) = -(a.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0));
	a2.at<double>(3,1) = a.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0);
	a2.at<double>(2,3) = -(a.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0));
	a2.at<double>(3,2) = a.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0);

	b2.at<double>(0,1) = -(b.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0));
	b2.at<double>(1,0) = b.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0);
	b2.at<double>(0,2) = -(b.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0));
	b2.at<double>(2,0) = b.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0);
	b2.at<double>(0,3) = -(b.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0));
	b2.at<double>(3,0) = b.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0);
	b2.at<double>(1,2) = b.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0);
	b2.at<double>(2,1) = -(b.at<double>(2,1) + a.at<double>(2,3) - q.at<double>(2,0));
	b2.at<double>(1,3) = -(b.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0));
	b2.at<double>(3,1) = b.at<double>(1,1) + a.at<double>(1,3) - q.at<double>(1,0);
	b2.at<double>(2,3) = -(b.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0));
	b2.at<double>(3,2) = b.at<double>(0,1) + a.at<double>(0,3) - q.at<double>(0,0);

	//third set
	a3.at<double>(0,1) = -(a.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0));
	a3.at<double>(1,0) = a.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0);
	a3.at<double>(0,2) = -(a.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0));
	a3.at<double>(2,0) = a.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0);
	a3.at<double>(0,3) = -(a.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0));
	a3.at<double>(3,0) = a.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0);
	a3.at<double>(1,2) = a.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0);
	a3.at<double>(2,1) = -(a.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0));
	a3.at<double>(1,3) = -(a.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0));
	a3.at<double>(3,1) = a.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0);
	a3.at<double>(2,3) = -(a.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0));
	a3.at<double>(3,2) = a.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0);

	b3.at<double>(0,1) = -(b.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0));
	b3.at<double>(1,0) = b.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0);
	b3.at<double>(0,2) = -(b.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0));
	b3.at<double>(2,0) = b.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0);
	b3.at<double>(0,3) = -(b.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0));
	b3.at<double>(3,0) = b.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0);
	b3.at<double>(1,2) = b.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0);
	b3.at<double>(2,1) = -(b.at<double>(2,2) + a.at<double>(2,3) - q.at<double>(2,0));
	b3.at<double>(1,3) = -(b.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0));
	b3.at<double>(3,1) = b.at<double>(1,2) + a.at<double>(1,3) - q.at<double>(1,0);
	b3.at<double>(2,3) = -(b.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0));
	b3.at<double>(3,2) = b.at<double>(0,2) + a.at<double>(0,3) - q.at<double>(0,0);

	n = (a1.t() * b1) + (a2.t() * b2) + (a3.t() * b3);

	Mat n_com = Mat::zeros(4,4,CV_64FC2);// (Mat_< Vec2f >(4, 4) << n.at<double>(0,0), n.at<double>(0,1), n.at<double>(0,2), n.at<double>(0,3),
	for(int i = 0; i<4; i++)
	{
		for(int j = 0; j<4; j++)
		{
			n_com.at<Vec2d>(i,j)[0] = n.at<double>(i,j);
			n_com.at<Vec2d>(i,j)[1] = 0;
		}
	}
	//	Mat nzi = n_com - comIden;

	MatrixXcd eigenN;// = MatrixXcf::Random(4,4);
	cv2eigen(n_com,eigenN);
	//	cout << "Here is a random 4x4 matrix, A:" << endl << eigenN << endl << endl;
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
//			cout<<"2 equal numbers"<<endl;
			com_idx[count] = i;
			count++;
		}
//		cout<<"numbers are: " << com_idx[0]<< com_idx[1]<< com_idx[2]<<com_idx[3]<<endl;
	}
//	cout << "\nThe eigenvectors are:" << endl << ces.eigenvectors();
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
		Mat rtMat = (Mat_<double>(3,4) <<	rmat.at<double>(0,0), rmat.at<double>(0,1), rmat.at<double>(0,2), tvec.at<double>(0,0),
					rmat.at<double>(1,0), rmat.at<double>(1,1), rmat.at<double>(1,2), tvec.at<double>(1,0),
					rmat.at<double>(2,0), rmat.at<double>(2,1), rmat.at<double>(2,2), tvec.at<double>(2,0));
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
			x2 = rtMat;
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
			cout<<"current rotation:\n"<<rtMat<<endl;
			determineRotation(rtMat, x2);
			x2 = rtMat;
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

