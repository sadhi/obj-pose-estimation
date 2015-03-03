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

#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>

class CameraView : public wxWindow{
public:
	CameraView(wxWindow *frame, const wxPoint& pos, const wxSize& size);
	virtual ~CameraView();

	void CheckUpdate();
	void DrawCam( cv::Mat* img );

protected:
	int m_nWidth;
	int m_nHeight;
	wxBitmap	m_pBitmap;

	bool	m_bDrawing;
	bool	m_bNewImage;

private:
	void OnPaint( wxPaintEvent& event );
	void Draw( wxDC& dc );
};

#endif /* CAMERAVIEW_H_ */