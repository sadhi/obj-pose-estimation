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

#include "pictureCam_thorlabs.hpp"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

pictureCam_thorlabs::pictureCam_thorlabs(): Cuc480Dll()
{
	m_hu = NULL;
	Connect(DRIVER_DLL_NAME);
	m_hwnd = NULL;
	m_hwndNotify = NULL;
	m_hDisplay = NULL;			// handle to diplay window
	m_Ret = 0;			// return value of SDK functions
	m_nColorMode = 0;	// Y8/RGB16/RGB24/RGB32
	m_nBitsPerPixel = 0;// number of bits needed store one pixel
	m_nSizeX = 0;		// width of video
	m_nSizeY = 0;		// height of video
	m_pcImageMemory = 0;// image memory - pointer to buffer
	m_lMemoryId = 0;    // image memory - buffer ID
	m_nDispModeSel = 0;
}

pictureCam_thorlabs::~pictureCam_thorlabs()
{
	Disconnect();
}

int pictureCam_thorlabs::InitCamera (int nId, HWND hwnd)
{
	int nRet = IS_CANT_OPEN_DEVICE;
	HCAM hu = (HCAM) nId;

	if((m_hu == NULL) && IsLoaded())
	{
		nRet = is_InitCamera (&hu, hwnd);

		if (nRet == IS_SUCCESS)
		{
			m_hu = hu;
			m_hwnd = hwnd;

			// Disable Autoexit
			is_EnableAutoExit( m_hu, IS_DISABLE_AUTO_EXIT );
		}
	}

	return nRet;
}

int pictureCam_thorlabs::ExitCamera ()
{
	int nRet = IS_CANT_CLOSE_DEVICE;

	if(m_hu != NULL)
	{
		nRet = is_ExitCamera (m_hu);
		m_hu = NULL;
	}

	return nRet;
}

int pictureCam_thorlabs::GetColorDepth (int* plBpp, int* plColFormat)
{
	return is_GetColorDepth (m_hu, plBpp, plColFormat);
}


int pictureCam_thorlabs::CameraStatus (int lInfo, long lValue)
{
	return is_CameraStatus (m_hu, lInfo, (ULONG)lValue);
}


int pictureCam_thorlabs::GetCameraType ()
{
	return is_GetCameraType (m_hu);
}


int pictureCam_thorlabs::GetOsVersion ()
{
	return is_GetOsVersion ();
}


int pictureCam_thorlabs::SetBrightness (int lBright)
{
	return is_SetBrightness (m_hu, lBright);
}


int pictureCam_thorlabs::SetContrast (int lCont)
{
	return is_SetContrast (m_hu, lCont);
}


int pictureCam_thorlabs::SetGamma (int lGamma)
{
	return is_SetGamma (m_hu, lGamma);
}


int pictureCam_thorlabs::SetWhiteBalance( INT nMode )
{
	return is_SetWhiteBalance(m_hu, nMode);
}


int pictureCam_thorlabs::SetWhiteBalanceMultipliers(double dblRed, double dblGreen, double dblBlue )
{
	return is_SetWhiteBalanceMultipliers(m_hu, dblRed, dblGreen, dblBlue);
}


int pictureCam_thorlabs::SetColorMode (int lMode)
{
	return is_SetColorMode (m_hu, lMode);
}


int pictureCam_thorlabs::InitEvent (HANDLE hEv, int nWhich)
{
	return is_InitEvent (m_hu, hEv, nWhich);
}


int pictureCam_thorlabs::EnableEvent (int nWhich)
{
	return is_EnableEvent (m_hu, nWhich);
}


int pictureCam_thorlabs::DisableEvent (int nWhich)
{
	return is_DisableEvent (m_hu, nWhich);
}


int pictureCam_thorlabs::ExitEvent (int nWhich)
{
	return is_ExitEvent (m_hu, nWhich);
}


int pictureCam_thorlabs::EnableMessage (int nWhich, HWND hwnd)
{
	return is_EnableMessage(m_hu, nWhich, hwnd);
}


int pictureCam_thorlabs::EnableAutoExit (int nMode)
{
	return is_EnableAutoExit(m_hu, nMode);
}


int pictureCam_thorlabs::SetImageSize (int lWidth, int lHeight)
{
	return is_SetImageSize (m_hu, lWidth, lHeight);
}


int pictureCam_thorlabs::SetImagePos (int lPosX, int lPosY)
{
	return is_SetDisplayPos (m_hu, lPosX, lPosY);
}


int pictureCam_thorlabs::SetDisplayMode (int Mode)
{
	return is_SetDisplayMode (m_hu, Mode);
}


int pictureCam_thorlabs::SetDisplayWindow (HWND hWnd)
{
	return is_SetHwnd (m_hu, hWnd);
}


int pictureCam_thorlabs::SetDisplayKeyColor (int lKeyCol)
{
	return is_SetKeyColor (m_hu, (lKeyCol & 0x00FF0000) >> 16, (lKeyCol & 0x0000FF00) >> 8, lKeyCol & 0x000000FF);
}

/*
 * @param EXP - value that sets the exposure
 * @param *newExp - returns the actually set exposure
 *
 * the exposure range is 0.037 - 10122 ms
 */
int pictureCam_thorlabs::SetExposureTime(double EXP, double* newEXP )
{
	return is_SetExposureTime(m_hu, EXP, newEXP );
}

int pictureCam_thorlabs::UpdateDisplay ()
{
	return is_UpdateDisplay (m_hu);
}


int pictureCam_thorlabs::CaptureVideo (int lWait)
{
	return is_CaptureVideo (m_hu, lWait);
}

int pictureCam_thorlabs::StopLiveVideo (int lWait)
{
  return is_StopLiveVideo (m_hu, lWait);
}

int pictureCam_thorlabs::RenderBitmap (int nMemID, HWND hwnd, int nMode)
{
	return is_RenderBitmap(m_hu, nMemID, hwnd, nMode);
}

int pictureCam_thorlabs::GetCameraInfo (CAMINFO* pInfo)
{
	int nRet = IS_SUCCESS;

	if(pInfo != NULL)
		nRet = is_GetCameraInfo (m_hu, pInfo);
	else
		nRet = IS_NULL_POINTER;

	return nRet;
}

int pictureCam_thorlabs::GetSensorInfo( PSENSORINFO pInfo )
{
	return is_GetSensorInfo( m_hu, pInfo );
}

void pictureCam_thorlabs::GetMaxImageSize(INT *pnSizeX, INT *pnSizeY)
{
	// Check if the camera supports an arbitrary AOI
	INT nAOISupported = 0;
	BOOL bAOISupported = TRUE;
	if (is_ImageFormat(m_hu,
			IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED,
			(void*)&nAOISupported,
			sizeof(nAOISupported)) == IS_SUCCESS)
	{
		bAOISupported = (nAOISupported != 0);
	}

	if (bAOISupported)
	{
		// Get maximum image size
		SENSORINFO sInfo;
		is_GetSensorInfo (m_hu, &sInfo);
		*pnSizeX = sInfo.nMaxWidth;
		*pnSizeY = sInfo.nMaxHeight;
	}
	else
	{
		// Get image size of the current format
		*pnSizeX = is_SetImageSize(m_hu, IS_GET_IMAGE_SIZE_X, 0);
		*pnSizeY = is_SetImageSize(m_hu, IS_GET_IMAGE_SIZE_Y, 0);
	}
}

int pictureCam_thorlabs::SetImageMem (char* pcMem, int lID)
{
	return is_SetImageMem (m_hu, pcMem, lID);
}

int pictureCam_thorlabs::FreeImageMem (char* pcMem, int lID)
{
	return is_FreeImageMem (m_hu, pcMem, lID);
}

int pictureCam_thorlabs::GetActiveImageMem (char** ppcImgMem, int* plID)
{
	return is_GetActiveImageMem (m_hu, ppcImgMem, plID);
}

int pictureCam_thorlabs::AllocImageMem (int lWidth, int lHeight, int lBpp, char** ppcImgMem, int* plID)
{
	return is_AllocImageMem (m_hu, lWidth, lHeight, lBpp, ppcImgMem, plID);
}

int pictureCam_thorlabs::DirectRenderer(UINT nMode, void* pParam, UINT nSizeOfParam)
{
	return is_DirectRenderer(m_hu, nMode, pParam, nSizeOfParam);
}

int pictureCam_thorlabs::InitDisplayMode()
{
	if (!IsInit())
	{
		return IS_NO_SUCCESS;
	}

	if (m_pcImageMemory != NULL)
	{
		FreeImageMem(m_pcImageMemory, m_lMemoryId);
	}

	m_pcImageMemory = NULL;

	switch (m_nDispModeSel)
	{
	case e_disp_mode_direct3D:
		// if initializiing the direct draw mode succeeded then
		// leave the switch tree else run through to bitmap mode
		m_Ret = SetDisplayMode(IS_SET_DM_DIRECT3D);
		m_Ret = DirectRenderer(DR_ENABLE_SCALING, NULL, NULL);

		if(m_Ret == IS_SUCCESS )
		{
			// setup the color depth to the current VGA setting
			GetColorDepth( &m_nBitsPerPixel, &m_nColorMode );
			break;
		}
		else
		{
			m_nDispModeSel = e_disp_mode_bitmap;
			//                UpdateData(FALSE);
		}

	case e_disp_mode_bitmap:
		m_Ret = SetDisplayMode(IS_SET_DM_DIB);
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
		if (AllocImageMem( m_nSizeX, m_nSizeY, m_nBitsPerPixel, &m_pcImageMemory, &m_lMemoryId ) != IS_SUCCESS )
		{
			cout<<"Memory allocation failed!"<<endl;
		}
		else
		{
			cout<<"Memory allocated "<< m_nSizeX<<"x"<< m_nSizeY<<endl;
			cout<<"with "<< m_nBitsPerPixel<<" bits per pixel"<<endl;
			SetImageMem( m_pcImageMemory, m_lMemoryId );
		}
		break;
	}   // end switch(m_nDispModeSel)

	if(m_Ret == IS_SUCCESS )
	{
		// set the desired color mode
		SetColorMode( m_nColorMode );

		// set the image size to capture
		SetImageSize( m_nSizeX, m_nSizeY );

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
	m_Ret = InitCamera(0, m_hDisplay );        // init cam

	// continue when camera is sucessfully initialized
	if( m_Ret == IS_SUCCESS )
	{
		// retrieve original image size
		GetSensorInfo(&m_sInfo );

		GetMaxImageSize(&m_nSizeX, &m_nSizeY);

		m_Ret = InitDisplayMode();

		// enable the dialog based error report
		if( m_Ret != IS_SUCCESS )
		{
			cout<< "ERROR: Can not enable the automatic error report!" <<endl;
			return false;
		}

		// do the whitebalance once on the first acquisitioned image only on color cameras
		if( m_sInfo.nColorMode == IS_COLORMODE_BAYER )
			SetWhiteBalance(IS_SET_WB_AUTO_ENABLE);

		return true;
	}
	else
	{
		cout<<"No camera could be opened !"<<endl;
	}

	return false;
}

void pictureCam_thorlabs::setExposure(int value)
{
	double exposure = 0;
	switch (value)
	{
	case 0: exposure = 1; break;
	case 1: exposure = 2; break;
	case 2: exposure = 5; break;
	case 3: exposure = 10; break;
	case 4: exposure = 20; break;
	case 5: exposure = 39; break;
	case 6: exposure = 78; break;
	case 7: exposure = 156; break;
	case 8: exposure = 312; break;
	case 9: exposure = 625; break;
	case 10: exposure = 1250; break;
	case 11: exposure = 1250; break;
	case 12: exposure = 2500; break;
	case 13: exposure = 5000; break;
	case 14: exposure = 10000; break;
	}
	cout<<"spam"<<endl;
	double result = 0;
//	SetExposureTime(exposure, &result );
	is_SetFrameRate(m_hu, 20, &result);
	cout<<"exposure was set to "<< result <<endl;
}

static void on_trackbar(int i, void* p)
{
	pictureCam_thorlabs *pct =(pictureCam_thorlabs*) p;

	//	double e;
	pct->setExposure(i);
}

int main() {
	pictureCam_thorlabs *pct = new pictureCam_thorlabs();
	namedWindow( "Original", WINDOW_AUTOSIZE );
	namedWindow( "result", WINDOW_AUTOSIZE );
	int expo = 1;
	createTrackbar("exposure_slider", "Original", &expo, 14, on_trackbar, &pct);
	//	on_trackbar(expo, pct);
	pct->setHDisplay((HWND)cvGetWindowHandle("Original"));	//get the HWND
	pct->OpenCamera();
	Mat imgMat;
	IplImage* cv_image ;
	bool running = true;

	if( pct->IsInit() )
	{
		pct->CaptureVideo( IS_WAIT );	//sets continuous video capture
	}
	if(pct->getPcImageMemory() != NULL)
		pct->RenderBitmap(pct->getLMemoryId(), pct->getHDisplay(), IS_RENDER_FIT_TO_WINDOW);
	//application loop
	while(running)
	{
		if(pct->getPcImageMemory() != NULL)
			pct->RenderBitmap(pct->getLMemoryId(), pct->getHDisplay(), IS_RENDER_FIT_TO_WINDOW);
		switch(waitKey(1))
		{
		case 27:	//esc
			cout << "Esc key is pressed by user." << endl;
			cout << "Closing application." << endl;
			running = false;
			break;
		case 's':
			/*
			 * get an image from the image memory
			 * convert it to cv::mat
			 * display the image
			 */
			cv_image = cvCreateImageHeader(cvSize(1280,1024), IPL_DEPTH_8U, 3);
			cvSetData(cv_image, pct->getPcImageMemory(), cv_image->widthStep);
			imgMat = Mat(cv_image, false);
//			imgMat +=imgMat;
			imshow("result", imgMat);
			break;
		default:
			break;
		}
	}

	if( pct->IsInit() )
	{
		pct->EnableMessage( IS_FRAME, NULL );
		pct->StopLiveVideo( IS_WAIT );
		if( pct->getPcImageMemory() != NULL )
			pct->FreeImageMem( pct->getPcImageMemory(), pct->getLMemoryId() );

		pct->setPcImageMemory(NULL);
		pct->ExitCamera();
	}
	return 0;
}

