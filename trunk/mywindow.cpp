#include "mywindow.hpp"

MyWindow::MyWindow(const char * theName): name(theName){
	width = 320;

	smallerC = cvCreateImage(cvSize(320,240), 8, 3);
	smallerG = cvCreateImage(cvSize(320,240), 8, 1);
}

MyWindow::MyWindow(const char * theName, int theWidth): name(theName), width(theWidth){

}

MyWindow::~MyWindow(){
	cvReleaseImage(&smallerC);
	cvReleaseImage(&smallerG);
}

void MyWindow::showHalfSizeImage(IplImage * frame){
	if( frame->nChannels > 1){
		cvResize(frame, smallerC);
		cvShowImage(name, smallerC);
	}else{
		cvResize(frame, smallerG);
		cvShowImage(name, smallerG);
	}
}


void MyWindow::showImage(IplImage * frame){
	cvShowImage(name, frame);
}

void MyWindow::hide(){
	cvDestroyWindow(name);
}