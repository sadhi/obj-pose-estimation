/*
 * example_UI.hpp
 *
 *  Created on: 23 jan. 2015
 *      Author: sadhi
 */

#ifndef EXAMPLE_UI_HPP_
#define EXAMPLE_UI_HPP_

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class Worker;
class CameraView;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

};

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

    bool initialized = false;
    bool resizingAllowed = true;
private:
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
    Worker* 	m_pWorker;
    CameraView* m_view1;
    CameraView* m_view2;
    wxStaticBox *pLiveCameraBox;
    wxStaticBox *pCameraBox;
    wxPanel*	m_pMainPanel;
};

#endif /* EXAMPLE_UI_HPP_ */
