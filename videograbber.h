#pragma once

#include <cv.h>
#include <highgui.h>

#include <QString>

#include "framegrabber.h"

/**
 * Obiekt obslugujacy pobieranie klatek z filmu wideo.
 */
class VideoGrabber: public FrameGrabber
{
public:
	VideoGrabber(QString file);
	~VideoGrabber(void);

	virtual bool init();
	virtual bool init(int width, int height);

	virtual void stop();

	virtual IplImage * getNextFrame();
	virtual bool hasNextFrame();

	//void setVideoFile(char * file);

protected:
	QString filename;
	CvCapture* cap;
};
