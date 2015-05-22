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
EVT_SLIDER(ID_Slider, MyFrame::OnScroll)
//EVT_SIZE(MyFrame::OnSize)
//EVT_SIZING(MyFrame::OnResize)
wxEND_EVENT_TABLE()


// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------
// frame constructor
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame(NULL, wxID_ANY, title, pos, size,  wxMINIMIZE_BOX | \
		wxMAXIMIZE_BOX | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN )
{
	initialized = false;
	resizingAllowed = true;
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
	if(m_pWorker != NULL)
	{
		m_pWorker->Delete( );
		m_pWorker = NULL;
	}
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
	m_slider = new wxSlider(m_pMainPanel, ID_Slider, 0, -100, 100, wxPoint(-1,-1), wxSize(-1,-1), wxSL_VERTICAL | wxSL_LABELS | wxSL_INVERSE);
//	wxPanel * panel1 = new wxPanel(m_pMainPanel, -1);
	wxPanel * panel2 = new wxPanel(m_pMainPanel, -1);

	//create the 2 identical collums
	wxGridSizer *gsh = new wxGridSizer(1, 2, 5, 5);

	//create a horizontal box that will hold the items in col=1, row=1
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(m_view2, 0, wxEXPAND | wxSHAPED | wxALL, 10);
	hbox->Add(m_slider, 0, wxEXPAND | wxSHAPED | wxALL, 10);

	//create the 2 identical rows and add items to them
	wxGridSizer *gsv1 = new wxGridSizer(2, 1, 5, 5);
	gsv1->Add(m_view1, 0, wxEXPAND | wxSHAPED | wxALL, 10);
	gsv1->Add(hbox,0, wxEXPAND , 10);

	//create the 3 identical rows and add items to them
	wxGridSizer *gsv2 = new wxGridSizer(3, 1, 5, 5);
	gsv2->Add(m_hist,0, wxEXPAND | wxSHAPED | wxALL, 10);

	xTxt1 = new wxStaticText(m_pMainPanel, -1, wxT("0"));
	xTxt2 = new wxStaticText(m_pMainPanel, -1, wxT("0"));
	yTxt1 = new wxStaticText(m_pMainPanel, -1, wxT("0"));
	yTxt2 = new wxStaticText(m_pMainPanel, -1, wxT("0"));
	zTxt1 = new wxStaticText(m_pMainPanel, -1, wxT("0"));
	zTxt2 = new wxStaticText(m_pMainPanel, -1, wxT("0"));


	wxGridSizer * gsd = new wxGridSizer(3, 4, 3, 3);
	//row 1
	gsd->Add(new wxStaticText(m_pMainPanel, -1, wxT("")), 0, wxEXPAND);
	gsd->Add(new wxStaticText(m_pMainPanel, -1, wxT("X")), 0, wxEXPAND);
	gsd->Add(new wxStaticText(m_pMainPanel, -1, wxT("Y")), 0, wxEXPAND);
	gsd->Add(new wxStaticText(m_pMainPanel, -1, wxT("Z")), 0, wxEXPAND);
	//row 2
	gsd->Add(new wxStaticText(m_pMainPanel, -1, wxT("Item 1")), 0, wxEXPAND);
	gsd->Add(xTxt1, 0, wxEXPAND);
	gsd->Add(yTxt1, 0, wxEXPAND);
	gsd->Add(zTxt1, 0, wxEXPAND);
	//row 3
	gsd->Add(new wxStaticText(m_pMainPanel, -1, wxT("Item 2")), 0, wxEXPAND);
	gsd->Add(xTxt2, 0, wxEXPAND);
	gsd->Add(yTxt2, 0, wxEXPAND);
	gsd->Add(zTxt2, 0, wxEXPAND);

	gsv2->Add(gsd,0, wxEXPAND);
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
	SetStatusText("Closing");
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

//	wxMessageBox("This should start a measurement",
//			"About wxWidgets minimal sample",
//			wxOK | wxICON_INFORMATION,
//			this);
	if(m_pWorker != NULL)
	{
		m_pWorker->Run();
		std::cout<<"test"<<std::endl;
		int i =  m_view2->GetSize().y/2 - m_slider->GetValue();
		m_view2->setLineHight(i);

	}
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

void MyFrame::OnScroll(wxCommandEvent& event)
{
	int i =  m_view2->GetSize().y/2 - m_slider->GetValue();
	m_view2->setLineHight(i);
	if(m_pWorker != NULL)
	{
		if(m_pWorker->IsRunning())
			m_pWorker->getROI()->y = i;
	}
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

void MyFrame::setX1Txt(wxString s)
{
	xTxt1->SetLabel(s);
	Layout();
}

void MyFrame::setY1Txt(wxString s)
{
	yTxt1->SetLabel(s);
	Layout();
}

void MyFrame::setZ1Txt(wxString s)
{
	zTxt1->SetLabel(s);
	Layout();
}

void MyFrame::setX2Txt(wxString s)
{
	xTxt2->SetLabel(s);
	Layout();
}

void MyFrame::setY2Txt(wxString s)
{
	yTxt2->SetLabel(s);
	Layout();
}

void MyFrame::setZ2Txt(wxString s)
{
	zTxt2->SetLabel(s);
	Layout();
}

wxSlider* MyFrame::getSlider()
{
	return m_slider;
}
