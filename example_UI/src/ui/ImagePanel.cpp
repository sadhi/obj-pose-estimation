/*
 * ImagePanel.cpp
 *
 *  Created on: 4 mrt. 2015
 *      Author: sadhi
 */

#include "ImagePanel.h"
#include <wx/wx.h>
#include <wx/sizer.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc.hpp"

/*
 * https://wiki.wxwidgets.org/An_image_panel
 */
BEGIN_EVENT_TABLE(ImagePanel, wxWindow)
// some useful events
/*
 EVT_MOTION(ImagePanel::mouseMoved)
 EVT_LEFT_DOWN(ImagePanel::mouseDown)
 EVT_LEFT_UP(ImagePanel::mouseReleased)
 EVT_RIGHT_DOWN(ImagePanel::rightClick)
 EVT_LEAVE_WINDOW(ImagePanel::mouseLeftWindow)
 EVT_KEY_DOWN(ImagePanel::keyPressed)
 EVT_KEY_UP(ImagePanel::keyReleased)
 EVT_MOUSEWHEEL(ImagePanel::mouseWheelMoved)
 */

// catch paint events
EVT_PAINT(ImagePanel::paintEvent)
//Size event
EVT_SIZE(ImagePanel::OnSize)
END_EVENT_TABLE()


// some useful events
/*
 void ImagePanel::mouseMoved(wxMouseEvent& event) {}
 void ImagePanel::mouseDown(wxMouseEvent& event) {}
 void ImagePanel::mouseWheelMoved(wxMouseEvent& event) {}
 void ImagePanel::mouseReleased(wxMouseEvent& event) {}
 void ImagePanel::rightClick(wxMouseEvent& event) {}
 void ImagePanel::mouseLeftWindow(wxMouseEvent& event) {}
 void ImagePanel::keyPressed(wxKeyEvent& event) {}
 void ImagePanel::keyReleased(wxKeyEvent& event) {}
 */

ImagePanel::ImagePanel(wxWindow *frame, const wxPoint& pos, const wxSize& size ):
wxWindow(frame, -1, pos, size, wxSIMPLE_BORDER )
{
	//    image.LoadFile(file, format);
    w = -1;
    h = -1;
    running = false;
}

ImagePanel::~ImagePanel( )
{

}

void ImagePanel::setGrayImage(cv::Mat grayImg)
{
	cv::Mat rgbOutput;	//output image
	//	file.convertTo(grayOutput,CV_8U);
	cvtColor(grayImg, rgbOutput, cv::COLOR_GRAY2BGR); // note the BGR here.
	//If on linux, set as RGB
	image = wxImage(rgbOutput.rows, rgbOutput.cols, rgbOutput.data, true);
}

void ImagePanel::setColorImage(cv::Mat img)
{
//	cv::Mat rgbOutput;	//output image
	//	file.convertTo(grayOutput,CV_8U);
//	cvtColor(grayOutput, rgbOutput, cv::COLOR_GRAY2BGR); // note the BGR here.
	//If on linux, set as RGB
	image = wxImage(img.rows, img.cols, img.data, true);
}

void ImagePanel::toggleRunning()
{
	if(running)
		std::cout<<"now running"<<std::endl;
	running = !running;
}

bool ImagePanel::isRunning()
{
	return running;
}

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

void ImagePanel::paintEvent(wxPaintEvent & evt)
{
	// depending on your system you may need to look at double-buffered dcs
	if(running)
	{
		wxPaintDC dc(this);
		render(dc);
	}
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void ImagePanel::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
	if(running)
	{
		wxClientDC dc(this);
		render(dc);
	}
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void ImagePanel::render(wxDC&  dc)
{
    int neww, newh;
    dc.GetSize( &neww, &newh );

    if( neww != w || newh != h )
    {
        resized = wxBitmap( image.Scale( neww, newh /*, wxIMAGE_QUALITY_HIGH*/ ) );
        w = neww;
        h = newh;
        dc.DrawBitmap( resized, 0, 0, false );
    }else{
        dc.DrawBitmap( resized, 0, 0, false );
    }
}

/*
 * Here we call refresh to tell the panel to draw itself again.
 * So when the user resizes the image panel the image should be resized too.
 */
void ImagePanel::OnSize(wxSizeEvent& event){
    Refresh();
    //skip the event.
    event.Skip();
}
//
//// ----------------------------------------
//// how-to-use example
//
//class MyApp: public wxApp
//{
//
//    wxFrame *frame;
//    ImagePanel * drawPane;
//public:
//    bool OnInit()
//    {
//        // make sure to call this first
//        wxInitAllImageHandlers();
//
//        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
//        frame = new wxFrame(NULL, wxID_ANY, wxT("Hello wxDC"), wxPoint(50,50), wxSize(800,600));
//
//        // then simply create like this
//        drawPane = new ImagePanel( frame, wxT("image.jpg"), wxBITMAP_TYPE_JPEG);
//        sizer->Add(drawPane, 1, wxEXPAND);
//
//        frame->SetSizer(sizer);
//
//        frame->Show();
//        return true;
//    }
//
//};
//
//IMPLEMENT_APP(MyApp)
