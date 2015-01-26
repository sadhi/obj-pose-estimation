/*
 * CameraView.h
 *
 *  Created on: 23 jan. 2015
 *      Author: sadhi
 */

#ifndef CAMERAVIEW_H_
#define CAMERAVIEW_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

class CameraView : public wxWindow{
public:
	CameraView(wxWindow *frame, const wxPoint& pos, const wxSize& size);
	virtual ~CameraView();

protected:
	int m_nWidth;
	int m_nHeight;
};

#endif /* CAMERAVIEW_H_ */
