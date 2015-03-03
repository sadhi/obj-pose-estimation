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
 *  |     |		designed it differently but made a mistake in the order and didn't want to change it again, sorry ...
 *  D --- C
 */
int getChessOrientation(Mat img)
{
	int boardHeight = 5;
	int boardWidth = 5;
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
		cout<<"\n<----------- Estimated pose -------------->"<<endl;
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

		//calculate the angle between the last point in the first row and the last point in the first collum.
		//this should be 90 degrees, however since my camera is angled at 45 degrees I should keep this into account
		Point2d p1, p2;
		p1.x = imagePoints[boardHeight-1].x - imagePoints[0].x;
		p1.y = imagePoints[boardHeight-1].y - imagePoints[0].y;
		p2.x = imagePoints[boardHeight*(boardWidth - 1)].x - imagePoints[0].x;
		p2.y = imagePoints[boardHeight*(boardWidth - 1)].y - imagePoints[0].y;
		double a = angleBetween(p1,p2);
		cout<<"angle is "<<a<<endl;
		if(a>80 && a<100)
		{
			//if-statement magic ....
			if(p1.x<20 && p1.x>-20)	//'long arm' vertical
			{
				if(p2.x>0 && p1.y<0)	//A,
				{
					cout<<"D1"<<endl;
					dvals[l] = a;
					l++;
				}
				else if(p2.x>0 && p1.y>0) //D,
				{
					cout<<"A1"<<endl;
					avals[u] = a;
					u++;
				}
				else if(p2.x<0 && p1.y<0)	//C,
				{
					cout<<"C1"<<endl;
					cvals[d] = a;
					d++;
				}
				else if(p2.x<0 && p1.y>0) //B,
				{
					cout<<"B1"<<endl;
					bvals[r] = a;
					r++;
				}
			}
			else	//'long arm' horizontal
			{
				if(p1.x>0 && p2.y<0)	//A, 'long arm' right, 'short arm' down
				{
					cout<<"D2"<<endl;
					dvals[l] = a;
					l++;
				}
				else if(p1.x>0 && p2.y>0) //D, 'long arm' right, 'short arm' up
				{
					cout<<"A2"<<endl;
					avals[u] = a;
					u++;
				}
				else if(p1.x<0 && p2.y<0)	//C, 'long arm' left, 'short arm' down
				{
					cout<<"C2"<<endl;
					cvals[d] = a;
					d++;
				}
				else if(p1.x<0 && p2.y>0) //B, 'long arm' left, 'short arm' up
				{
					cout<<"B2"<<endl;
					bvals[r] = a;
					r++;
				}
			}
			ndx++;
			//find the camera extrinsic parameters
			//If the distortion is NULL/empty, the zero distortion coefficients are assumed
			line(img, imagePoints[0],imagePoints[boardHeight-1],w);
			line(img, imagePoints[0],imagePoints[boardHeight*(boardWidth - 1)],w);
			//		rectangle(img, imagePoints[0], imagePoints[imagePoints.size()-1],CV_RGB(255,255,255));
			solvePnPRansac( Mat(boardPoints), Mat(imagePoints), intrinsics, distortion, rvec, tvec, false ,200,8.0,200);
			//			//show the pose estimation data
			cout << fixed << setprecision(4) << "rvec = ["
					<< rvec.at<double>(0,0) << ", "
					<< rvec.at<double>(1,0) << ", "
					<< rvec.at<double>(2,0) << "] \t" << "tvec = ["
					<< tvec.at<double>(0,0) << ", "
					<< tvec.at<double>(1,0) << ", "
					<< tvec.at<double>(2,0) << "]" << endl;
			Mat rotationMatrix;
			Rodrigues(rvec,rotationMatrix);
			cout << "\nrotation matrix:\n["
					<< rotationMatrix.at<double>(0,0) << ", "
					<< rotationMatrix.at<double>(1,0) << ", "
					<< rotationMatrix.at<double>(2,0) << "] \n["
					<< rotationMatrix.at<double>(0,1) << ", "
					<< rotationMatrix.at<double>(1,1) << ", "
					<< rotationMatrix.at<double>(2,1) << "] \n["
					<< rotationMatrix.at<double>(0,2) << ", "
					<< rotationMatrix.at<double>(1,2) << ", "
					<< rotationMatrix.at<double>(2,2) << "] \n"<< endl;
		}
		else
		{
			cout << "bad measurement"<<endl;
			badMeh++;
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
	//these do not need to be made in advance
	//	namedWindow( "result", WINDOW_AUTOSIZE );
	//	resizeWindow("result",50,1);
	//	namedWindow("histogram", WINDOW_AUTOSIZE );
	//	resizeWindow("histogram",50,1);

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

		//		Mat M = (Mat_<double>(3,3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
		distortion = (Mat_<double>(5,1) << 7.2914041963856420e+000, 2.0979596618049214e+000, -2.3690347009888141e-001, 5.1098820884635382e-002, 1.4682521220954941e-003 );
		intrinsics =  (Mat_<double>(3,3) << 2.0529114902590052e+004, 0., 6.3928941571612074e+002, 0.,
				2.5657205522615815e+004, 5.1836001554552104e+002, 0., 0., 1.) ;

		running = true;
	}
	/*
	 * load camera parameters
	 */
	//	FileStorage fs;
	//	fs.open(filename, FileStorage::READ);
	//	// read camera matrix and distortion coefficients from file
	//	fs["Camera_Matrix"] >> intrinsics;
	//	fs["Distortion_Coefficients"] >> distortion;
	//	// close the input file
	//	fs.release();

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

		//		equalizeHist( grayImg, grayImg );		//preferably not stretched with software
		calcHist(&grayImg);
		/* threshold type
		 * 0: Binary
		 * 1: Binary Inverted
		 * 2: Threshold Truncated
		 * 3: Threshold to Zero
		 * 4: Threshold to Zero Inverted
		 */
		//		threshold( grayImg, grayImg, 30, 255, CV_THRESH_BINARY );

		/*
		 * let's do some opening and closing
		 */
		//		element = getStructuringElement(MORPH_RECT, Size(4, 4) );
		//		dilate(grayImg, grayImg, element);
		//		dilate(grayImg, grayImg, element);
		//		//		element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5) );
		//		erode(grayImg, grayImg, element);
		//		erode(grayImg, grayImg, element);
		//		element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5) );
		//		erode(grayImg, grayImg, element);
		//		dilate(grayImg, grayImg, element);
		//		erode(grayImg, grayImg, element);
		//		dilate(grayImg, grayImg, element);
		//		componentAnalysis(imgMat);
		//		orbExample(grayImg, imgMat);
		getChessOrientation(grayImg);
		//cout<<ndx<<"+"<<endl;
		//		imshow("result", imgMat);
		//		detailEnhance(grayImg,grayImg);	//opencv-3.0.0 beta function
		imshow("result", grayImg);
		//		if(ndx >= 20)
		//			running =false;
		switch(waitKey(1))
		{
		case 27:	//esc
			cout << "Esc key is pressed by user." << endl;
			cout << "Closing application." << endl;
			running = false;
			break;
			//		case 's':
			//			/*
			//			 * check out this code: https://github.com/MasteringOpenCV/code
			//			 * from "Mastering OpenCV with Practical Computer Vision Projects by Daniel Lelis Baggio" http://image2measure.net/files/Mastering_OpenCV.pdf
			//			 * chapter 2 could be the answer to my problem
			//			 */
			//
			//
			//			getChessOrientation(grayImg);
			//			imshow("result", imgMat);
			//			break;
		default:
			break;
		}
		if(ndx>=200)
			running=false;
	}
	if(u!=0)
	{
		avg = 0;
		for(int i = 0; i<u; i++)
		{
			cout << fixed <<setprecision(4)<< avals[i]<<"\t";
			avg += avals[i];
		}
		cout<<"\nThe average angle for A is "<<avg/u<<"\n"<<endl;
	}

	if(r!=0)
	{
		avg = 0;
		for(int i = 0; i<r; i++)
		{
			cout << fixed <<setprecision(4)<< bvals[i]<<"\t";
			avg += bvals[i];
		}
		cout<<"\nThe average angle for B is "<<avg/r<<"\n"<<endl;
	}

	if(d!=0)
	{
		avg = 0;
		for(int i = 0; i<d; i++)
		{
			cout << fixed <<setprecision(4)<< cvals[i]<<"\t";
			avg += cvals[i];
		}
		cout<<"\nThe average angle for C is "<<avg/d<<"\n"<<endl;
	}

	if(l!=0)
	{
		avg = 0;
		for(int i = 0; i<l; i++)
		{
			cout << fixed <<setprecision(4)<< dvals[i]<<"\t";
			avg += dvals[i];
		}
		cout<<"\nThe average angle for D is "<<avg/l<<"\n"<<endl;
	}

	if(ndx!=0)
		cout << "there were "<< badMeh << " bad measurements"<<endl;

	pct->ExitCamera();
	pct = NULL;
	return 0;
}

