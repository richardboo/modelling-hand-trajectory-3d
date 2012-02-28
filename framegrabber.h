#pragma once

#include <cv.h>
#include <highgui.h>

/**
 * Klasa abstrakcyjna.
 * Zawiera interfejs obiektu pobierania klatek obrazu, np. kamery lub filmu wideo.
 */
class FrameGrabber
{
public:
	FrameGrabber(void);
	~FrameGrabber(void);

	virtual bool init() = 0;
	virtual bool init(int width, int height) = 0;

	virtual void stop() = 0;

	virtual IplImage * getNextFrame() = 0;
	virtual bool hasNextFrame() = 0;

	int getWidth()	{	return width;	}
	int getHeight()	{	return height;	}

protected:
	int width;
	int height;

	IplImage * img;
};
