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

#include <opencv2/core/core.hpp>
#include "ui/MyFrame.h"
#include "vision/pictureCam_thorlabs.hpp"

#include "opencv2/videoio/videoio_c.h"

class MyFrame;

class Worker : public wxThread{
public:
	Worker(MyFrame *frame);
	virtual ~Worker();

	// thread entry point
	virtual void *Entry();
	virtual void OnExit();
	void calcHist(cv::Mat*);
	void stop();
	cv::Rect *getROI();
	void determineRotation(cv::Mat, cv::Mat);

private:
	MyFrame			*m_pFrame;
	unsigned char	m_bLife;
	pictureCam_thorlabs *pct;
	IplImage* cv_image;
	cv::Mat distortion;
	cv::Mat intrinsics;

	double thetaX, thetaY, thetaZ;
	cv::Rect * r;
	cv::Mat rotPoint;
	cv::Mat m_a, m_b;

	void calibrate();
};

#endif /* WORKER_H_ */
