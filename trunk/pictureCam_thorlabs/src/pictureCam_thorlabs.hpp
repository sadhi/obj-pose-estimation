/*
 * pictureCam_thorlabs.hpp
 *
 *  Created on: 29 jan. 2015
 *      Author: sadhi
 */

#ifndef PICTURECAM_THORLABS_HPP_
#define PICTURECAM_THORLABS_HPP_

#include <windows.h>
#include "uc480.h"
//#   define DS_EXPORT
//#include "uc480CaptureInterface.h"
//#include <initguid.h>
#include "Dynamic_uc480.h"

typedef enum _disp_mode
{
  e_disp_mode_bitmap = 0,
  e_disp_mode_direct3D
};

class pictureCam_thorlabs : public Cuc480Dll//, Iuc480CapturePin
{
public:
	pictureCam_thorlabs();
	virtual ~pictureCam_thorlabs();

	HCAM  GetCameraHandle ()          { return m_hu; }
	HWND  GetWindowHandle ()          { return m_hwnd; }
	HWND  GetNotifyWindowHandle ()    { return m_hwndNotify; }
	BOOL  IsInit()                    { return m_hu != NULL; }
	int   EnableDeviceStateNotification(BOOL boEnable, HWND hwnd = NULL);

	// init/exit functions
	int   InitCamera            (int nId = 0, HWND hwnd = NULL);
	int   ExitCamera            ();
	int   SetErrorReport        (int lMode);
	int   GetErrorText          (int lErr, char* pcErrText);
	bool OpenCamera();
	int  InitDisplayMode();

	// inquire functions
	int   GetNumberOfCameras    (int* plNumCameras);
	int   GetDllVersion         (int* plDllVers);
	int   GetPciSlot            (int* plSlot);
	int   GetIRQ                (int* plIRQ);
	int   GetColorDepth         (int* plBpp, int* plColFormat);
	int   CameraStatus          (int lInfo, long lValue);
	int   GetCameraType         ();
	int   GetOsVersion          (void);

	// image parameter functions
	int   SetBrightness               (int lBright);
	int   SetContrast                 (int lCont);
	int   SetGamma                    (int lGamma);
	int   SetSaturation               (int lSatU, int lSatV);
	int   SetHue                      (int lHue);
	int   SetColorMode                (int lMode);
	int   SetWhiteBalance             (int nMode );
	int   SetWhiteBalanceMultipliers  (double dblRed, double dblGreen, double dblBlue );

	// callback functions
	int   InitEvent             (HANDLE hEv, int nWhich);
	int   EnableEvent           (int nWhich);
	int   ExitEvent             (int nWhich);
	int   DisableEvent          (int nWhich);
	int   EnableAutoExit        (int nMode);
	int   EnableMessage         (int nWhich, HWND hwnd);

	// display functions
	int   SetImageSize          (int lWidth, int lHeight);
	int   SetImagePos           (int lPosX, int lPosY);
	int   SetScrollPos          (int lPosX, int lPosY);
	int   SetDisplayOffset      (int lOffsetX, int lOffsetY);
	int   SetDisplayMode        (int Mode);
	int   SetDisplayWindow      (HWND hWnd);
	int   SetDisplayKeyColor    (int lKeyCol);
	int   SetMainDisplayWindow  (HWND hWnd);
	int   UpdateDisplay         ();
	int   SetDisplayHook        (int lMode);
	int   SetScaler             (int nMode);

	// video capture functions
	int   CaptureVideo   (int lWait);
	int   StopLiveVideo  (int lWait);
	int   FreezeVideo    (int lWait);
	int   CaptureFrame   (BITMAPINFOHEADER* pbmih, char* pcImage);
	int   SetCaptureMode (int lMode);

	// memory handling functions
	int   AllocImageMem     (int lWidth, int lHeight, int lBpp, char** ppcImgMem, int* plID);
	int   FreeImageMem      (char* pcMem, int lID);
	int   SetImageMem       (char* pcMem, int lID);
	int   GetActiveImageMem (char** ppcImgMem, int* plID);
	int   InquireImageMem   (char* pcMem, int lID, int* plX, int* plY, int* plBpp, int* plPitch);

	// digital I/O and trigger input functions
	int   ReadTriggerPort    (int* plIn);
	int   ReadDigitalPort    (int* plIn);
	int   WriteDigitalPort   (int lOut);
	int   SetExternalTrigger (int lMode);

	// EEPROM access functions
	int   GetCameraInfo (CAMINFO* pInfo);
	int   ReadEEPROM    (int lAdr, char* pcBuf, int lCount);
	int   WriteEEPROM   (int lAdr, char* pcBuf, int lCount);

	// bitmap load and save functions
	int   SaveImage    (char* pcFile);
	int   LoadImage    (char* pcFile);
	int   SaveImageMem (char* pcFile, char* pcMem, int lID);

	// bitmap render functions
	int   RenderBitmap (int nMemID, HWND hwnd, int nMode);

	// timing functions
	int   GetPixelClockRange	( int *nMin, int *nMax );
	int   SetPixelClock(int nClock);
	int   SetFrameRate(double dFr, double* pdFrNew);
	int   SetExposureTime(double EXP, double* newEXP );

	int   Renumerate( int reserved );
	int   GetSensorInfo( PSENSORINFO pInfo );
	int   LoadParameters( char* pFilename);

	int   DirectRenderer(UINT nMode, void* pParam, UINT nSizeOfParam);

	void  GetMaxImageSize(INT *pnSizeX, INT *pnSizeY);
	void setExposure(int);

	/*
	 * virtual stuff from uc480CaptureInterface
	 */
	/*!
//		 *  \brief  Returns the sum (in MHz) of pixelclock.
//		 *	\param 	lClock      Receives the overall pixelclock sum.
//		 *	\return	HRESULT     0 on success, error code otherwise.
//		 *	\see    GetPixelClock
//		 */
//		long GetUsedBandwith(long *lClock);
//
//	    /*!
//	    *   \brief  Returns the pixelclock for the connected camera.
//	    *	\param 	plClock     Receives the current pixel clock.
//	    *	\return	HRESULT     0 on success, error code otherwise.
//	    *	\see    GetPixelClockRange, SetPixelClock
//	    */
//		long GetPixelClock(long *plClock);
//
//	    /*!
//		* \brief	Returns the min, max and default value for the pixelclock.
//	     *	\param 	plMin       Receives the minimum possible pixel clock.
//	     *	\param 	plMax       Receives the maximum possible pixel clock.
//		*\param	plDefault   	Receives the default pixel clock value.
//		*\return	HRESULT     0 on success, error code otherwise.
//		*\see    	GetPixelClock, SetPixelClock
//	     */
//		long GetPixelClockRange(long *plMin, long *plMax, long *plDefault);
//
//	    /*!
//		* \brief	Sets the Pixelclock (in MHz) for the connected device.
//	     *	\param 	lClock      The pixel clock in MHz to set.
//	     *	\return	HRESULT     0 on success, CO_E_NOT_SUPPORTED if not supported, error code otherwise.
//	     *	\see    GetPixelClock, GetPixelClockRange
//	     */
//		long SetPixelClock(long lClock);
//
//	    /*!
//		* \brief	Queries which color mode to use when RGB8 mediatype is selected.
//	     *	\param 	plMode      Receives the currently selected RGB8 colormode.
//	     *	\return	HRESULT     0 on success, error code otherwise.
//	     *	\see    SetRGB8ColorMode
//	     */
//		long GetRGB8ColorMode(long *plMode);
//
//	    /*!
//		* \brief	Determines which color mode to use when RGB8 mediatype is selected.
//		*
//	     *      Specifies whether Y8 or raw bayer pattern is used with RGB8 mode
//	     *		possible values are 11 for raw bayer pattern (on bayer color
//	     *      cameras), or 6 for monochrome images.
//	     *	\param 	lMode       Specifies the color mode used for RGB8 mode.
//	     *	\return	HRESULT     0 on success, error code otherwise.
//	     *	\see    GetRGB8ColorMode
//	     */
//		long SetRGB8ColorMode(long lMode);
//
//		/*!
//		* \brief	Queries the current possible min, max and interval for exposure time.
//		*
//	     *      Gets the actual min, max and interval values for exposure time.
//	     *      Values are given in us unit.
//		 *	\param 	plMinExp    Receives the minimum possible exposure time.
//		 *	\param 	plMaxExp    Receives the maximum possible exposure time.
//	     *	\param 	plInterval  Receives the current possible step width.
//	     *	\return	HRESULT     0 on success, error code otherwise.
//		 *	\see    GetExposureTime, SetExposureTime, SetPixelClock
//	     *
//	     *  \note   This range may change depending on framerate and pixelclock
//	     *          settings.
//		 */
//		long GetExposureRange(long *plMinExp, long *plMaxExp, long *plInterval);
//
//	    /*!
//		* \brief	Queries the current exposure time
//	     *	\param 	plExp       Receives the current exposure time in us.
//	     *	\return	HRESULT     0 on success, error code otherwise.
//	     *	\see    GetExposureRange, SetExposureTime
//	     */
//	    long GetExposureTime(long *plExp );
//
//	    /*!
//		* \brief	Sets the exposure time of the camera.
//	     *		This function sets the exposure time in units of Microseconds and
//	     *      thus allows a finer exposure time granularity than the function of
//	     *      the IAMCameraControl Interface does. (2^n seconds vs. x us).
//	     *	\param 	lExp        Specifies the exposure time to use (in us).
//	     *	\return	HRESULT     0 on success, error code otherwise.
//	     *	\see    GetExposureTime, GetExposureRange
//	     */
//	    long SetExposureTime(long lExp);

	HWND getHDisplay() const {
		return m_hDisplay;
	}

	void setHDisplay(HWND hDisplay) {
		m_hDisplay = hDisplay;
	}

	HCAM getHu() const {
		return m_hu;
	}

	void setHu(HCAM hu) {
		m_hu = hu;
	}

	HWND getHwnd() const {
		return m_hwnd;
	}

	void setHwnd(HWND hwnd) {
		m_hwnd = hwnd;
	}

	INT getNSizeX() const {
		return m_nSizeX;
	}

	void setNSizeX(INT nSizeX) {
		m_nSizeX = nSizeX;
	}

	INT getNSizeY() const {
		return m_nSizeY;
	}

	void setNSizeY(INT nSizeY) {
		m_nSizeY = nSizeY;
	}

	INT getRet() const {
		return m_Ret;
	}

	void setRet(INT ret) {
		m_Ret = ret;
	}

	char* getPcImageMemory() const {
		return m_pcImageMemory;
	}

	void setPcImageMemory(char* pcImageMemory) {
		m_pcImageMemory = pcImageMemory;
	}

	INT getLMemoryId() const {
		return m_lMemoryId;
	}

	void setLMemoryId(INT lMemoryId) {
		m_lMemoryId = lMemoryId;
	}

protected:
	HCAM  	m_hu;
	HWND  	m_hwnd;
	HWND  	m_hwndNotify;
	HWND	m_hDisplay;			// handle to diplay window
	INT		m_Ret;			// return value of SDK functions
	INT		m_nColorMode;	// Y8/RGB16/RGB24/RGB32
	INT		m_nBitsPerPixel;// number of bits needed store one pixel
	INT		m_nSizeX;		// width of video
	INT		m_nSizeY;		// height of video
	char*   m_pcImageMemory;// image memory - pointer to buffer
	INT     m_lMemoryId;    // image memory - buffer ID
	SENSORINFO m_sInfo;     // sensor info struct
	int 	m_nDispModeSel;
};

#endif /* PICTURECAM_THORLABS_HPP_ */
