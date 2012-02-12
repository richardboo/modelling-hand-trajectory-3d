#include "bgmask.hpp"
#include "settings.h"

BGMask::BGMask(void){
	foregroundSmaller = cvCreateImage(Settings::instance()->defSmallSize, 8, 1);
}

BGMask::~BGMask(void){
	cvReleaseImage(&foregroundSmaller);
	cvReleaseImage(&imageGray);
	cvReleaseImage(&imageDiff);
	cvReleaseImage(&bigImageDiff);
	cvReleaseImage(&onlyForeground);
}

void BGMask::init(int resize, IplImage * frame){

	CvSize imageSize = cvGetSize(frame);

	foreground = cvCreateImage(imageSize, 8, 1);
	cvCvtColor(frame, foreground, CV_BGR2GRAY);

	imageGray = cvCreateImage(imageSize, 8, 1);
	imageDiff = cvCreateImage(imageSize, 8, 1);
	bigImageDiff = cvCreateImage(imageSize, 8, 1);

	onlyForeground = cvCreateImage(imageSize, 8, 3);

	alpha = 0.07f;

	
	//foregroundSmaller = cvCreateImage(cvSize(imageSize.width/resize, imageSize.height/resize), 8, 1);
}



void BGMask::update(IplImage * frame, IplImage * imageGray, IplImage * nobg){

	//cvCvtColor(frame, imageGray, CV_BGR2GRAY);

	//if(framesCounter % 15 == 0)
	cvAddWeighted(imageGray, alpha, foreground, (1.0-alpha), 0.0, foreground);
	
	cvAbsDiff(imageGray, foreground, imageDiff);

	cvThreshold(imageDiff, imageDiff, 15, 255, CV_THRESH_BINARY);
	
	cvZero(nobg);
	cvCopy(frame, nobg, imageDiff);
	
/*
	cvErode(imageDiff, imageDiff);
	cvDilate(imageDiff, imageDiff, NULL, 5);
	cvErode(imageDiff, imageDiff, NULL, 4);
	*/
}

IplImage * BGMask::getFrameForeground(){
	cvResize(imageDiff, foregroundSmaller);
	return foregroundSmaller;
}