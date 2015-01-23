/*
 * Worker.h
 *
 *  Created on: 23 jan. 2015
 *      Author: sadhi
 */

#ifndef WORKER_H_
#define WORKER_H_

#include <wx/wxprec.h>
#include <wx/thread.h>

#ifndef  WX_PRECOMP
  #include <wx/wx.h>
#endif //precompiled headers

#include "example_UI.hpp"

class Worker : public wxThread{
public:
	Worker(MyFrame *frame);
	virtual ~Worker();

	// thread entry point
	virtual void *Entry();
	virtual void OnExit();

private:
	MyFrame			*m_pFrame;
	unsigned char	m_bLife;
};

#endif /* WORKER_H_ */
