#include "imageutils.hpp"

IplImage * ImageUtils::getGray(IplImage * frame){

	IplImage * frameGray = cvCreateImage(cvGetSize(frame), 8, 1);
	cvCvtColor(frame, frameGray, CV_BGR2GRAY);
	return frameGray;
}


void ImageUtils::getGray(IplImage * frame, IplImage * gray){
	cvCvtColor(frame, gray, CV_BGR2GRAY);
}

IplImage * ImageUtils::createGray(CvSize size){
	IplImage * frameGray = cvCreateImage(size, 8, 1);
	return frameGray;
}

IplImage * ImageUtils::create(CvSize size){
	IplImage * frame = cvCreateImage(size, 8, 3);
	return frame;
}

