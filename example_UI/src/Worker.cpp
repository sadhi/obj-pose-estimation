/*
 * Worker.cpp
 *
 *  Created on: 23 jan. 2015
 *      Author: sadhi
 */

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// wxwidgets includes
#include "wx/wxprec.h"
#include "wx/thread.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

// main header include
#include "Worker.h"

Worker::Worker(MyFrame *frame) : wxThread( wxTHREAD_DETACHED ){
	// TODO Auto-generated constructor stub
	m_pFrame = frame;
	// start life
	m_bLife = 1;

	return;
}

Worker::~Worker() {
	// TODO Auto-generated destructor stub
}

////////////////////////////////////////////////////////////////////
// Method:	On Exit
// Class:	CwxopencvWorker
// Purpose:	do soemthing on exit
// Input:	nothing
// Output:	nothing
////////////////////////////////////////////////////////////////////
void Worker::OnExit( )
{
	// destroy - clean my place
}

////////////////////////////////////////////////////////////////////
// Method:		Entry
// Class:		CwxopencvWorker
// Purpose:		the main executable body of my thread
// Input:		nothing
// Output:		void pointer
////////////////////////////////////////////////////////////////////
void *Worker::Entry( )
{
	m_bLife = 1;

	////////////////////////////////////////////////////////////////
	// Start Life Cycle
	////////////////////////////////////////////////////////////////
	// loop as long as flag m_bLife = 1
	while( m_bLife )
	{
		//TODO: from here I want to do most of the things I have to do
		// can be compared to the Main-loop of my application
		// this is because it is not a nice way to do it from example_UI
		wxThread::Sleep( 5 );

	}

    return NULL;
}
