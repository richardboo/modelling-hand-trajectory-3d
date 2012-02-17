#pragma once

#include <opencv/cv.h>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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
	void drawPointsOnDisp(IplImage * disparityToShow, std::vector<cv::Point2f> & mpts_1, Blob * hand);

private:
	CvFont font;
	char strInt[10];
};