/*
 * CameraView.cpp
 *
 *  Created on: 23 jan. 2015
 *      Author: sadhi
 */

#include <wx/wxprec.h>

#ifndef  WX_PRECOMP
  #include <wx/wx.h>
#endif //precompiled headers

#include "../example_UI.hpp"
#include "CameraView.h"

CameraView::CameraView(wxWindow *frame, const wxPoint& pos, const wxSize& size ):
wxWindow(frame, -1, pos, size, wxSIMPLE_BORDER )
{
	// set my canvas width/height
		m_nWidth = size.GetWidth( );
		m_nHeight = size.GetHeight( );

}

CameraView::~CameraView() {
	// TODO Auto-generated destructor stub
}

