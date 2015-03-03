/*
 * MyFrame1.h
 *
 *  Created on: 3 mrt. 2015
 *      Author: sadhi
 */

#ifndef UI_MYFRAME_H_
#define UI_MYFRAME_H_

#include "../Worker.h"
#include "CameraView.h"

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,
	//TODO: define new values for these 2:
	Minimal_Start = wxID_ZOOM_IN,
	Minimal_Stop = wxID_ZOOM_OUT,
    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT

};

class Worker;

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size );
    virtual ~MyFrame();

    void createMenu();
    void createGUIContent();

    void setWorker(Worker* w);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnResize(wxSizeEvent& event);
    CameraView* getLiveWindow();
    CameraView* getGrayWindow();
    CameraView* getHistWindow();

    bool initialized = false;
    bool resizingAllowed = true;

private:
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
    Worker* 	m_pWorker;
    CameraView* m_view1;
    CameraView* m_view2;
    CameraView* m_hist;
    wxSlider*	m_slider;
    wxPanel*	m_pMainPanel;
};


#endif /* UI_MYFRAME1_H_ */
