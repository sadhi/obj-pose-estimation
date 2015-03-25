/*
 * ImagePanel.h
 *
 *  Created on: 4 mrt. 2015
 *      Author: sadhi
 */

#ifndef UI_IMAGEPANEL_H_
#define UI_IMAGEPANEL_H_

#include <wx/wx.h>
#include <wx/sizer.h>
#include <opencv2/core/core.hpp>

class ImagePanel : public wxWindow
{
protected:
    wxImage image;
    wxBitmap resized;
    bool running;
    int w, h;
	int m_nWidth;
	int m_nHeight;

public:
    ImagePanel(wxWindow *frame, const wxPoint& pos, const wxSize& size);
    virtual ~ImagePanel();

    void paintEvent(wxPaintEvent & evt);
    void paintNow();
    void setColorImage(cv::Mat);
    void setGrayImage(cv::Mat);
    void toggleRunning();
    bool isRunning();
    void OnSize(wxSizeEvent& event);
    void render(wxDC& dc);

    // some useful events
    /*
     void mouseMoved(wxMouseEvent& event);
     void mouseDown(wxMouseEvent& event);
     void mouseWheelMoved(wxMouseEvent& event);
     void mouseReleased(wxMouseEvent& event);
     void rightClick(wxMouseEvent& event);
     void mouseLeftWindow(wxMouseEvent& event);
     void keyPressed(wxKeyEvent& event);
     void keyReleased(wxKeyEvent& event);
     */

    DECLARE_EVENT_TABLE()
};

#endif /* UI_IMAGEPANEL_H_ */
