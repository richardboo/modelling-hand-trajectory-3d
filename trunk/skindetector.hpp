#pragma once

#include "histogram.hpp"
#include "blob.hpp"

class SkinDetector
{
public:
	SkinDetector();
	~SkinDetector();

	Histogram skinHistogram;
	bool skinFound;

	void updateHistogram(IplImage * frame, CvRect & rect);

	void detectSkin(IplImage * frame, IplImage * skin, CvRect & rect, int type, IplImage * fg);

	void detectSkinRGB(IplImage * frame, IplImage * skin, CvRect & rect);
	void detectSkinHSV(IplImage * frame, IplImage * skin, CvRect & rect);
	void detectSkinHist(IplImage * frame, IplImage * skin, CvRect & rect);
	void detectSkinAll(IplImage * frame, IplImage * skin, CvRect & rect);
	void detectSkinBg(IplImage * frame, IplImage * skin, CvRect & rect, IplImage * fg);
	void detectSkinBgR(IplImage * frame, IplImage * skin, CvRect & rect, IplImage * fg);

	void detectSkinYCRCB(IplImage * frame, IplImage * skin, CvRect & rect);
/*
	void detectSkinFromHistogram(IplImage * frame, CvRect & rect);
	void detectSkinRGB(IplImage * frame, CvRect & rect);
	void detectSkinHSV(IplImage * frame, CvRect & rect);
*/
	int getSampleCount(){ return framesCounter; }

private:
	IplImage * maskTemp;
	IplImage * temp;
	IplImage * skinMask;

	IplImage * channel0, * channel1, * channel2;

	int framesCounter;
};
