/*
 * MyFrame1.cpp
 *
 *  Created on: 3 mrt. 2015
 *      Author: sadhi
 */
// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "MyFrame.h"

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
//EVT_SIZE(MyFrame::OnSize)
//EVT_SIZING(MyFrame::OnResize)
wxEND_EVENT_TABLE()


// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------
//TODO: might have to put MyFrame in a separate file
// frame constructor
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame(NULL, wxID_ANY, title, pos, size,  wxMINIMIZE_BOX | \
		wxMAXIMIZE_BOX | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN )
{
	SetMaxClientSize(wxSize(1920,1080));
	SetMinClientSize(wxSize(1024, 720));
	Maximize(true);
	createMenu();
	createGUIContent();

#if wxUSE_STATUSBAR
	// create a status bar just for fun (by default with 1 pane only)
	CreateStatusBar(2);
	SetStatusText("Welcome to wxWidgets!");
#endif // wxUSE_STATUSBAR
	Layout();
}

MyFrame::~MyFrame( )
{
	// tell first to the worker to exit
	m_pWorker->Delete( );
	m_pWorker = NULL;

//	m_view1 = NULL;
//	m_view2 = NULL;
//	pLiveCameraBox = NULL;
//	pCameraBox = NULL;
//	pMainSizer1->DeleteWindows();
//	pMainSizer2->DeleteWindows();
//	pMainSizer1 = NULL;
//	pMainSizer2 = NULL;
//	m_pMainPanel = NULL;
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
void MyFrame::createGUIContent()
{
	int width, height;
	this->GetClientSize(&width, &height);

	//create the panel on which everything will be placed
	m_pMainPanel = new wxPanel(this, -1);

	//create a boxsizer that will hold our layout
	//items added to this will be added vertically
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	m_view1 = new CameraView( m_pMainPanel, wxPoint(-1,-1), wxSize(1280, 1024) );
	m_view2 = new CameraView(  m_pMainPanel, wxPoint(-1,-1), wxSize(1280, 1024) );
	m_hist = new CameraView(  m_pMainPanel, wxPoint(-1,-1), wxSize(1024, 400) );
	m_slider = new wxSlider(m_pMainPanel, -1, 0, -100, 100, wxPoint(-1,-1), wxSize(-1,-1), wxSL_VERTICAL | wxSL_LABELS | wxSL_INVERSE);
	wxPanel * panel1 = new wxPanel(m_pMainPanel, -1);
	wxPanel * panel2 = new wxPanel(m_pMainPanel, -1);

	//create the 2 identical collums
	wxGridSizer *gsh = new wxGridSizer(1, 2, 5, 5);

	//create a horizontal box that will hold the items in col=1, row=1
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(m_view1, 0, wxEXPAND | wxSHAPED | wxALL, 10);
	hbox->Add(m_slider, 0, wxEXPAND | wxSHAPED | wxALL, 10);

	//create the 2 identical rows and add items to them
	wxGridSizer *gsv1 = new wxGridSizer(2, 1, 5, 5);
	gsv1->Add(hbox, 0, wxEXPAND );
	gsv1->Add(m_view2,0, wxEXPAND | wxSHAPED | wxALL, 10);

	//create the 3 identical rows and add items to them
	wxGridSizer *gsv2 = new wxGridSizer(3, 1, 5, 5);
	gsv2->Add(m_hist,0, wxEXPAND | wxSHAPED | wxALL, 10);
	gsv2->Add(panel1,0, wxEXPAND);
	gsv2->Add(panel2,0, wxEXPAND);

	//add items to the collums
	gsh->Add(gsv1, 0, wxEXPAND);
	gsh->Add(gsv2, 0, wxEXPAND);


	vbox->Add(gsh, 0, wxEXPAND);

	m_pMainPanel->SetSizer(vbox);

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
	if(m_pWorker != NULL)
		m_pWorker->Run();
	else
		std::cout<<"No active workerthread"<<std::endl;
}

void MyFrame::OnStop(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(
			"This should stop a measurement",
			"About wxWidgets minimal sample",
			wxOK | wxICON_INFORMATION,
			this);
//	m_pWorker->Exit();
}


void MyFrame::setWorker(Worker* w)
{
	m_pWorker = w;
}

CameraView* MyFrame::getLiveWindow()
{
	return m_view1;
}

CameraView* MyFrame::getGrayWindow()
{
	return m_view2;
}

CameraView* MyFrame::getHistWindow()
{
	return m_hist;
}
