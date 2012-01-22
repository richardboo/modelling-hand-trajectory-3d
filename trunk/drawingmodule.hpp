#pragma once

#include <cv.h>
#include "blob.hpp"

class DrawingModule{
public:
	DrawingModule();
	~DrawingModule();

	void drawFPSonFrame(int fps, IplImage * frame);
	void drawRectOnFrame(CvRect & rect, IplImage * frame);
	void drawSmallerRectOnFrame(CvRect & rect, IplImage * frame, CvScalar & color);
	void drawSamplesOnFrame(int samples, IplImage * frame);
	void drawTextOnFrame(char * text, IplImage * frame);
	void drawDispOnFrame(int disp, IplImage * dispIm, IplImage * dispToShow);
	void drawMiddleOnFrame(Blob * hand, IplImage * frame);
	void drawTrajectoryOnFrame(Blob * hand, IplImage * frame);

private:
	CvFont font;
	char strInt[10];
};