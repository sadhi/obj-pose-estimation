/*
 * pictureCam_thorlabs.hpp
 *
 *  Created on: 29 jan. 2015
 *      Author: sadhi
 */

#ifndef PICTURECAM_THORLABS_HPP_
#define PICTURECAM_THORLABS_HPP_

#include <windows.h>
#include "uc480class.h"

enum _disp_mode
{
	e_disp_mode_bitmap = 0,
	e_disp_mode_direct3D
};

class pictureCam_thorlabs
{
protected:
	Cuc480   m_camera;

	// Camera varibles
	HWND	m_hDisplay;			// handle to diplay window
	INT		m_Ret;			// return value of SDK functions
	INT		m_nColorMode;	// Y8/RGB16/RGB24/RGB32
	INT		m_nBitsPerPixel;// number of bits needed store one pixel
	INT		m_nSizeX;		// width of video
	INT		m_nSizeY;		// height of video
	char*   m_pcImageMemory;// image memory - pointer to buffer
	INT     m_lMemoryId;    // image memory - buffer ID
	SENSORINFO m_sInfo;     // sensor info struct
	bool m_cameraLoaded;
	int m_nDispModeSel;

public:
	pictureCam_thorlabs(HWND);
	virtual ~pictureCam_thorlabs();

	// Implementation

	bool OpenCamera();
	void ExitCamera();
	int  InitDisplayMode();
	void render();
	void setExposure(int value);
	void setFps(int value);
	void setPixelClock(int value);

	//general getters/setters
	HWND getHDisplay();
	void setHDisplay(HWND hDisplay) ;
	INT getNSizeX();
	void setNSizeX(INT nSizeX);
	INT getNSizeY();
	void setNSizeY(INT nSizeY);
	INT getRet();
	void setRet(INT ret) ;
	char* getPcImageMemory();
	void setPcImageMemory(char* pcImageMemory);
	INT getLMemoryId();
	void setLMemoryId(INT lMemoryId);
	Cuc480 getCamera();

	HCAM  GetCameraHandle ();
	HWND  GetWindowHandle ();
	HWND  GetNotifyWindowHandle ();
	BOOL  IsInit();

};

#endif /* PICTURECAM_THORLABS_HPP_ */
