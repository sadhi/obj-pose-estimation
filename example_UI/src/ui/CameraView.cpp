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

#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core/types.hpp>

using namespace cv;

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

////////////////////////////////////////////////////////////////////
// Method:	OnPaint
// Class:	CameraView
// Purose:	on paint event
// Input:	reference to paint event
// Output:	nothing
////////////////////////////////////////////////////////////////////
void CameraView::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);
	Draw( dc );
}

////////////////////////////////////////////////////////////////////
// Method:	Draw
// Class:	CameraView
// Purose:	camera drawing
// Input:	reference to dc
// Output:	nothing
////////////////////////////////////////////////////////////////////
void CameraView::Draw( wxDC& dc )
{
	// check if dc available
	if( !dc.IsOk() || m_bDrawing == true ){ return; }

		m_bDrawing = true;

//		dc.BeginDrawing();	//Deprecated, did nothing anyway?

		int x,y,w,h;
		dc.GetClippingBox( &x, &y, &w, &h );
		// if there is a new image to draw
		if( m_bNewImage )
		{
			dc.DrawBitmap( m_pBitmap, x, y );
			m_bNewImage = false;
		} else
		{
			// draw inter frame ?
		}

//		dc.EndDrawing();	//Deprecated, did nothing anyway?
		m_bDrawing = false;

	return;
}

////////////////////////////////////////////////////////////////////
// Method:	OnDraw
// Class:	CameraView
// Purose:	CCamView drawing
// Input:	nothing
// Output:	nothing
////////////////////////////////////////////////////////////////////
void CameraView::DrawCam( Mat* img )
{
//	return;
	if( m_bDrawing ) return;
	m_bDrawing = true;
	int i = 0;

	// if there was an image then we need to update view
    if( img )
    {
        Mat pDstImg = img->clone(); //m_Canvas.GetImage();

		int nCamWidth = m_nWidth;
		int nCamHeight = m_nHeight;

		// draw a rectangle
		rectangle( pDstImg,
				 	Point(10,10),
					Point( nCamWidth-10, nCamHeight-10 ),
					Scalar( 0,255,0 ) );


		// process image from opencv to wxwidgets
		wxImage pWxImg = wxImage( nCamWidth, nCamHeight, pDstImg.data, TRUE );
		// convert to bitmap to be used by the window to draw
		m_pBitmap = wxBitmap( pWxImg.Scale(m_nWidth, m_nHeight) );

		m_bNewImage = true;
		m_bDrawing = false;

		Refresh( FALSE );

		Update( );

		pDstImg = NULL;

    }

}

////////////////////////////////////////////////////////////////////
// Method:	CheckUpdate
// Class:	CameraView
// Purose:	CHeck for updates
// Input:	reference to size event
// Output:	nothing
////////////////////////////////////////////////////////////////////
void CameraView::CheckUpdate()
{
	Update( );
}
