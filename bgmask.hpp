#pragma once

#include <cv.h>
//#include <opencv2/video/background_segm.hpp>

/**
 * Klasa tworzaca maske tla i pierwszego planu.
 */
class BGMask
{
public:
	BGMask(void);
	~BGMask(void);

	void update(IplImage * frame, IplImage * grayImage, IplImage * nobg);
	void init(int resize, IplImage * frame);

	IplImage * foregroundSmaller;
	//CvBGStatModel* bgModel;
	IplImage * getFrameForeground();
	IplImage * imageDiff;

	IplImage * onlyForeground;

private:
	float alpha;
	IplImage * foregroundColor;
	IplImage * lastFrame;

	IplImage * foreground;
	IplImage * imageGray;
	
	IplImage * bigImageDiff;
};
