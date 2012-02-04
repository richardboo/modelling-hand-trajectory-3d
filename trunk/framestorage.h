#pragma once

#include <cv.h>

// aktualnie przetwarzana ramka i jej odpowiedniki (mniejsze, szare itp)
class FrameStorage
{
public:
	FrameStorage(void);
	~FrameStorage(void);

	void initFrames();

	IplImage * frame[2], * frameSmaller[2], *frameShow[2];
	IplImage * frameBackground[2];
	IplImage * frameDiff[2];
	IplImage * frameRectified[2];
	IplImage * frameGray[2];
	IplImage * frameSmallerGray[2];
	IplImage * frameSkin[2];
	IplImage * frameBlob[2];

	IplImage * blackImage;

	IplImage * disparity;
	IplImage * disparitySmaller;
	IplImage * disparityToShow;

	IplImage * trajectory;
	IplImage * trajectorySmaller;

private:
	void releaseFames();
};
