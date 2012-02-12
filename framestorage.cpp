#include "framestorage.h"
#include "settings.h"

FrameStorage::FrameStorage(void)
{
	for(int i = 0; i < 2; ++i){
		frame[i] = NULL;
		frameSmaller[i] = NULL;
		frameShow[i] = NULL;
		frameDiff[i] = NULL;
		frameRectified[i] = NULL;
		frameBackground[i] = NULL;
		frameGray[i] = NULL;
		frameSmallerGray[i] = NULL;
		frameSkin[i] = NULL;
		frameBlob[i] = NULL;
	}
	blackImage = NULL;
	disparity = NULL;
	disparitySmaller = NULL;
	disparityToShow = NULL;
	trajectory = NULL;
	trajectorySmaller = NULL;



}

FrameStorage::~FrameStorage(void){
	
}

void FrameStorage::initFrames(){
	
	CvSize size = Settings::instance()->defSize;
	CvSize smaller = Settings::instance()->defSmallSize;

	for(int i = 0; i < 2; ++i){
		frame[i] = cvCreateImage(size, 8, 3);
		frameSmaller[i] = cvCreateImage(smaller, 8, 3);
		frameSmallerGray[i] = cvCreateImage(smaller, 8, 1);

		frameShow[i] = cvCreateImage(size, 8, 3);
		frameDiff[i] = cvCreateImage(size, 8, 1);
		frameBackground[i] = cvCreateImage(size, 8, 1);
		cvZero(frameBackground[i]);
		frameRectified[i] = cvCreateImage(size, 8, 3);

		frameGray[i] = cvCreateImage(size, 8, 1);
		frameSkin[i] = cvCreateImage(size, 8, 1);
		frameBlob[i] = cvCreateImage(size, 8, 1);
	}

	blackImage = cvCreateImage(smaller, 8, 1);
	cvZero(blackImage);

	disparity = cvCreateImage(size, IPL_DEPTH_8U, 1);
	disparitySmaller = cvCreateImage(smaller, 8, 1);
	disparityToShow = cvCreateImage(smaller, 8, 3);
	cvZero(disparityToShow);

	trajectory = cvCreateImage(size, 8, 1);
	trajectorySmaller = cvCreateImage(smaller, 8, 1);
	cvZero(trajectorySmaller);

}

void FrameStorage::releaseFames(){
	if(blackImage == NULL)
		return;

	for(int i = 0; i < 2; ++i){
		cvReleaseImage(&frame[i]);
		cvReleaseImage(&frameSmaller[i]);
		cvReleaseImage(&frameSmallerGray[i]);
		cvReleaseImage(&frameShow[i]);
		cvReleaseImage(&frameDiff[i]);
		cvReleaseImage(&frameBackground[i]);
		cvReleaseImage(&frameRectified[i]);
		cvReleaseImage(&frameGray[i]);
		cvReleaseImage(&frameSkin[i]);
		cvReleaseImage(&frameBlob[i]);
	}

	cvReleaseImage(&blackImage);
	cvReleaseImage(&disparity);
	cvReleaseImage(&disparitySmaller);
	cvReleaseImage(&disparityToShow);
	cvReleaseImage(&trajectory);
	cvReleaseImage(&trajectorySmaller);
}