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
#include <iostream>
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
#include "ui/MyFrame.h"
#include "Worker.h"

using namespace std;


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
//	wxInitAllImageHandlers();
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
	frame->initialized = true;

	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned false here, the
	// application would exit immediately.
	return true;
}
