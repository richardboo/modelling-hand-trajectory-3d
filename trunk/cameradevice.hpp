#pragma once

#include <cv.h>
#include <highgui.h>
#include <videoInput.h>

#include "framegrabber.h"

/**
 * Obiekt reprezentujacy pojedyncza kamere.
 */
class CameraDevice: public FrameGrabber
{
public:
	CameraDevice(int id);

	~CameraDevice();

	virtual bool init();
	virtual bool init(int width, int height);

	virtual void stop();

	virtual IplImage * getNextFrame();
	virtual bool hasNextFrame();

	int getId()		{	return id;		}

	static videoInput vi;

private:
	

	int id;
	bool initialized;
};

