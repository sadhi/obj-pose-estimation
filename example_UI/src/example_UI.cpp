//============================================================================
// Name        : example_UI.cpp
// Author      : Bas Koevoets
// Version     : v0.1
// Copyright   : Build on minimal wxWidgets sample by Julian Smart
// Description : example of what the final UI might look like
//============================================================================

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#include "wx/thread.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "example_UI.hpp"
#include "worker.h"
#include "ui/CameraView.h"

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
EVT_MENU(Minimal_About, MyFrame::OnAbout)
EVT_MENU(Minimal_Start, MyFrame::OnStart)
EVT_MENU(Minimal_Stop, MyFrame::OnStop)
wxEND_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	// call the base class initialization method, currently it only parses a
	// few common command-line options but it could be do more in the future
	if ( !wxApp::OnInit() )
		return false;

	// create the main application window
	MyFrame *frame = new MyFrame("Minimal wxWidgets App", wxPoint(-1, -1), wxSize(1280, 720));
//	frame->SetSize(1280, 720);
	Worker* w = new Worker( frame );
	// create thread or fail on exit
	if ( w->Create() != wxTHREAD_NO_ERROR )
	{
		wxExit( );
	}
	frame->setWorker(w);
	// and show it (the frames, unlike simple controls, are not shown when
	// created initially)
	frame->Show(true);

	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned false here, the
	// application would exit immediately.
	return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------
//TODO: might have to put MyFrame in a separate file
// frame constructor
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame(NULL, wxID_ANY, title, pos, size,  wxMINIMIZE_BOX | \
		wxMAXIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
	SetMaxClientSize(wxSize(1920,1080));
	SetMinClientSize(wxSize(1024, 720));
//	Maximize(true);
	createMenu();
	createGUIContend();

#if wxUSE_STATUSBAR
	// create a status bar just for fun (by default with 1 pane only)
	CreateStatusBar(2);
	SetStatusText("Welcome to wxWidgets!");
#endif // wxUSE_STATUSBAR
}

MyFrame::~MyFrame( )
{
	// tell first to the worker to exit
	m_pWorker->Delete( );

	m_pWorker = NULL;
}

void MyFrame::createMenu()
{
	// create a menu bar
	wxMenu *fileMenu = new wxMenu;

	// the "About" item should be in the help menu
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");

	fileMenu->Append(Minimal_Start, "S&tart\tAlt-S", "Start camera capture");
	fileMenu->Append(Minimal_Stop, "St&op\tAlt-P", "Stop camera capture");
	fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit this program");

	// now append the freshly created menu to the menu bar...
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(helpMenu, "&Help");

	// ... and attach this menu bar to the frame
	SetMenuBar(menuBar);
}

//TODO: this stuff should dynamically change depending on the screen resolution!!!
void MyFrame::createGUIContend()
{
	int width, height;
	this->GetClientSize(&width, &height);

	m_pMainPanel = new wxPanel(this, -1, wxPoint(0,0), wxSize(width,height), 0 );
	// build static/logical boxes

	// build static/logical boxes
	wxStaticBox *pLiveCameraBox = new wxStaticBox( m_pMainPanel, -1, "Live camera",
			wxPoint(5,5), wxSize(400,315) );

	wxStaticBox *pCameraBox = new wxStaticBox( m_pMainPanel, -1, "Extra camera info",
				wxPoint(5,325), wxSize(400,315) );

	// get my main static sizer by the box
	wxStaticBoxSizer *pMainSizer1 = new wxStaticBoxSizer( pLiveCameraBox, wxHORIZONTAL );
	wxStaticBoxSizer *pMainSizer2 = new wxStaticBoxSizer( pCameraBox, wxHORIZONTAL );

	m_view1 = new CameraView( m_pMainPanel, wxPoint(15,25), wxSize(380, 285) );
	m_view2 = new CameraView( m_pMainPanel, wxPoint(15,345), wxSize(380, 285) );
	pMainSizer1->Add( m_view1, 1, wxALIGN_NOT );
	pMainSizer2->Add( m_view2, 1, wxALIGN_NOT );

//	m_pMainPanel->SetSizer( pMainSizer );
	pMainSizer1->SetSizeHints( m_pMainPanel );
	m_pMainPanel->SetAutoLayout( TRUE );


	wxBoxSizer *pTopSizer = new wxBoxSizer(wxVERTICAL);
	pTopSizer->Add( m_pMainPanel, 1, wxALIGN_NOT |wxEXPAND, 0 );
	SetSizer( pTopSizer );

	// display my stuff
	SetAutoLayout( TRUE );
}

// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	// true is to force the frame to close
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(wxString::Format
			(
					"Welcome to %s!\n"
					"\n"
					"This is the sample user interface\n"
					"of what the final result might look like\n"
					"running under %s.",
					wxVERSION_STRING,
					wxGetOsDescription()
			),
			"About wxWidgets minimal sample",
			wxOK | wxICON_INFORMATION,
			this);
}

void MyFrame::OnStart(wxCommandEvent& WXUNUSED(event))
{

	wxMessageBox("This should start a measurement",
			"About wxWidgets minimal sample",
			wxOK | wxICON_INFORMATION,
			this);
}

void MyFrame::OnStop(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(
			"This should stop a measurement",
			"About wxWidgets minimal sample",
			wxOK | wxICON_INFORMATION,
			this);
}

void MyFrame::setWorker(Worker* w)
{
	m_pWorker = w;
}
