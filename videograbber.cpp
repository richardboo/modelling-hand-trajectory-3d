#include "videograbber.h"

VideoGrabber::VideoGrabber(QString file): filename(file){
	cap = NULL;
}

VideoGrabber::~VideoGrabber(void){
	if(cap)
		cvReleaseCapture(&cap);
}

bool VideoGrabber::init(){
	cap = cvCaptureFromAVI(filename.toAscii().data());
	return cap != NULL;
}

bool VideoGrabber::init(int width, int height){
	return true;
}
/*
void VideoGrabber::setVideoFile(char * file){
	filename = file;
}
*/
void VideoGrabber::stop(){

}

IplImage * VideoGrabber::getNextFrame(){
	return img;
}

bool VideoGrabber::hasNextFrame(){
	img = cvQueryFrame(cap);
	return img != NULL;
}
