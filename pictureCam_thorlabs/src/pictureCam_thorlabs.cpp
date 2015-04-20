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

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>

using namespace cv;
using namespace std;

//probably should not define them here like this, but it makes it a lot easier for me
pictureCam_thorlabs *pct;
double badMeh, avg;
int ndx, u, d;
int l, r;
string filename = "out_camera_data.yml";
Mat intrinsics, distortion;
double avals[200] = {};
double bvals[200] = {};
double cvals[200] = {};
double dvals[200] = {};
Mat tvecOld1, tvecOld2;

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
		//		if(found)
		//			cornerSubPix(img, imagePoints, Size(5, 5), Size(-1, -1),
		//					TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
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
		double a = angleBetween(p1,p2);
		//		cout<<"angle is "<<a<<endl;
		//		if(a>70 && a<110)
		//		{
		vector<Point2d> imgPoints;
		//			//if-statement magic ....
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
					imgPoints[i-1] = imagePoints[(boardHeight*boardWidth)-i];
				}
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
		//		rectangle(img, imagePoints[0], imagePoints[imagePoints.size()-1],CV_RGB(255,255,255));
		solvePnPRansac( Mat(boardPoints), Mat(imgPoints), intrinsics, distortion, rvec, tvec, false ,200,8.0,200);
		//			//			//show the pose estimation data
//		tvec.at<double>(0,0) = roundf(tvec.at<double>(0,0) * 10000) / 10000;
//		tvec.at<double>(1,0) = roundf(tvec.at<double>(1,0) * 10000) / 10000;
//		tvec.at<double>(2,0) = roundf(tvec.at<double>(2,0) * 10000) / 10000;
//		cout <<fixed<<setprecision(4)<<  "tvec = "<< tvec << endl;
//		cout << "\ntvecOld1 = "<< tvecOld1<< endl;

		solvePnPRansac( Mat(boardPoints), Mat(imagePoints), intrinsics, distortion, rvec, tvecO, false ,200,8.0,200);
//		tvecO.at<double>(0,0) = roundf(tvecO.at<double>(0,0) * 10000) / 10000;
//		tvecO.at<double>(1,0) = roundf(tvecO.at<double>(1,0) * 10000) / 10000;
//		tvecO.at<double>(2,0) = roundf(tvecO.at<double>(2,0) * 10000) / 10000;
//		cout << "\n\ntvecO = "<< tvecO<< endl;
//		cout << "\ntvecOld2 = "<< tvecOld2<< endl;

		if(ndx==0)
		{
			tvecOld1 = tvec;
			tvecOld2 = tvecO;
			avals[ndx] = tvec.at<double>(0,0) ;//- tvecOld2.at<double>(0,0);
			bvals[ndx] = tvec.at<double>(1,0) ;//- tvecOld2.at<double>(1,0);
			cvals[ndx] = tvec.at<double>(2,0) ;//- tvecOld2.at<double>(2,0);

		}
		else
		{
			cout<<"\nindex = "<<ndx<<endl;
			double d1 = pow(pow(tvec.at<double>(0,0) - tvecOld1.at<double>(0,0),2) + pow(tvec.at<double>(1,0) - tvecOld1.at<double>(1,0),2) + pow(tvec.at<double>(2,0) - tvecOld1.at<double>(2,0),2), 0.5);
			d1 = roundf(d1 * 100) / 100;	//round to 2 decimals
			cout<<"tvec , tvecOld1 = " << d1 << endl;
			double d2 = pow(pow(tvec.at<double>(0,0) - tvecOld2.at<double>(0,0),2) + pow(tvec.at<double>(1,0) - tvecOld2.at<double>(1,0),2) + pow(tvec.at<double>(2,0) - tvecOld2.at<double>(2,0),2), 0.5);
			d2 = roundf(d2 * 100) / 100;	//round to 2 decimals
			cout<<"tvec , tvecOld2 = " << d2 << endl;

			double d3 = pow(pow(tvecO.at<double>(0,0) - tvecOld1.at<double>(0,0),2) + pow(tvecO.at<double>(1,0) - tvecOld1.at<double>(1,0),2) + pow(tvecO.at<double>(2,0) - tvecOld1.at<double>(2,0),2), 0.5);
			d3 = roundf(d3 * 100) / 100;	//round to 2 decimals
			cout<<"\ntvecO , tvecOld1 = " << d3 << endl;
			double d4 = pow(pow(tvecO.at<double>(0,0) - tvecOld2.at<double>(0,0),2) + pow(tvecO.at<double>(1,0) - tvecOld2.at<double>(1,0),2) + pow(tvecO.at<double>(2,0) - tvecOld2.at<double>(2,0),2), 0.5);
			d4 = roundf(d4 * 100) / 100;	//round to 2 decimals
			cout<<"tvecO , tvecOld2 = " << d4 << endl;

			if(d4>d3 && d1>2)	//a different origin was taken when compared to the last measurement
			{
				avals[ndx] = tvec.at<double>(0,0) ;//- tvecOld2.at<double>(0,0);
				bvals[ndx] = tvec.at<double>(1,0) ;//- tvecOld2.at<double>(1,0);
				cvals[ndx] = tvec.at<double>(2,0) ;//- tvecOld2.at<double>(2,0);

				tvecOld1 = tvecO;
				tvecOld2 = tvec;
			}
			else	//same origin
			{
				avals[ndx] = tvec.at<double>(0,0) ;//- tvecOld1.at<double>(0,0);
				bvals[ndx] = tvec.at<double>(1,0) ;//- tvecOld1.at<double>(1,0);
				cvals[ndx] = tvec.at<double>(2,0) ;//- tvecOld1.at<double>(2,0);

				tvecOld1 = tvec;
				tvecOld2 = tvecO;
			}
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
	ndx = u = d = l = r =  0;
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

		distortion = (Mat_<double>(5,1) << 3.8654777889913724e+001, -3.8245991379523409e-001,
				3.9884910642621724e-001, 1.4881952204747112e-001,
				-1.0394776844313427e-004 );
		intrinsics =  (Mat_<double>(3,3) << 2.4321094911390839e+004, 0., 4.8782427879579103e+002, 0.,
				3.0850178156461727e+004, 4.0423527036496580e+002, 0., 0., 1.) ;

		running = true;
	}
	bool printP = false;

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

		imshow("result", grayImg);
		if(ndx == 200)
			running =false;
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
			//			/*
			//			 * check out this code: https://github.com/MasteringOpenCV/code
			//			 * from "Mastering OpenCV with Practical Computer Vision Projects by Daniel Lelis Baggio" http://image2measure.net/files/Mastering_OpenCV.pdf
			//			 * chapter 2 could be the answer to my problem
			//			 */
			//
			//
			//			getChessOrientation(grayImg);
			//			imshow("result", imgMat);
			break;
		default:
			break;
		}
		//		if(ndx>=200)
		//			running=false;
	}

//	if(u!=0)
//	{
		avg = 0;
	cout<<"\nThe difference in x is:\n"<<endl;
		for(int i = 0; i<ndx-1; i++)
		{
			cout << fixed <<setprecision(4)<< avals[i]<<"\t";
			avg += avals[i];
		}
		cout<<"\nThe average difference in x is "<<avg/(ndx-1)<<"\n"<<endl;
//	}
//
//	if(r!=0)
//	{
		avg = 0;
		cout<<"\nThe difference in y is:\n"<<endl;
		for(int i = 0; i<ndx-1; i++)
		{
			cout << fixed <<setprecision(4)<< bvals[i]<<"\t";
			avg += bvals[i];
		}
		cout<<"\nThe average difference in y is "<<avg/(ndx-1)<<"\n"<<endl;
//	}
//
//	if(d!=0)
//	{
		avg = 0;
		cout<<"\nThe difference in z is:\n"<<endl;
		for(int i = 0; i<ndx-1; i++)
		{
			cout << fixed <<setprecision(4)<< cvals[i]<<"\t";
			avg += cvals[i];
		}
		cout<<"\nThe average difference in z is "<<avg/(ndx-1)<<"\n"<<endl;
//	}
//
//	if(l!=0)
//	{
//		avg = 0;
//		for(int i = 0; i<l; i++)
//		{
//			cout << fixed <<setprecision(4)<< dvals[i]<<"\t";
//			avg += dvals[i];
//		}
//		cout<<"\nThe average angle for D is "<<avg/l<<"\n"<<endl;
//	}
//
//	if(ndx!=0)
//		cout << "there were "<< badMeh << " bad measurements"<<endl;

	pct->ExitCamera();
	pct = NULL;
	return 0;
}

