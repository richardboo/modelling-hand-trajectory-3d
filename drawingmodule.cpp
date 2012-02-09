#include "drawingmodule.hpp"

#include <iostream>
#include <qDebug>

using namespace std;

DrawingModule::DrawingModule(){
	font = cvFont(1.5, 2); 
}

DrawingModule::~DrawingModule(){

}

void DrawingModule::drawTextOnFrame(char * text, IplImage * frame){
	cvPutText(frame, text, cvPoint(30, 30), &font, cvScalar(0, 0, 200));
}

void DrawingModule::drawDispOnFrame(int disp, IplImage * dispIm, IplImage * dispToShow){
/*
	cvZero(dispToShow);

	int red, green;
	int middle = 128 - disp;

	if(middle <= 0){
		green = 255 - 2*abs(middle);
		red = 255 - abs(middle);
	}else{
		red = 255 - 2*abs(middle);
		green =  255 - abs(middle);
	}

	cvSet(dispToShow, cvScalar(0, green, red), dispIm);
*/
	sprintf(strInt, "odleglosc: %d", disp);
	cvPutText(dispIm, strInt, cvPoint(30, 30), &font, cvScalar(255, 255, 255));
}

void DrawingModule::drawRectOnFrame(CvRect & rect, IplImage * frame){
	cvDrawRect(frame, cvPoint(rect.x, rect.y), cvPoint(rect.x+rect.width, rect.y+rect.height),
			cvScalar(200, 0, 0));
}

void DrawingModule::drawSmallerRectOnFrame(CvRect & rect, IplImage * frame, CvScalar & color){
	cvDrawRect(frame, cvPoint(rect.x/2, rect.y/2), cvPoint(rect.x/2+rect.width/2, rect.y/2+rect.height/2), color);
}


void DrawingModule::drawFPSonFrame(int fps, IplImage * frame){
	sprintf(strInt, "%d fps", fps);
	cvPutText(frame, strInt, cvPoint(30, 30), &font, cvScalar(0, 0, 200));
}

void DrawingModule::drawSamplesOnFrame(int samples, IplImage * frame){
	sprintf(strInt, "%d probek", samples);
	cvPutText(frame, strInt, cvPoint(10, 400), &font, cvScalar(0, 200, 0));
}

void DrawingModule::drawMiddleOnFrame(Blob * hand, IplImage * frame){
	if(hand->lastPoint.x <= 0)
		return;

	cvCircle(frame, hand->lastPoint, 3, cvScalarAll(100), 3);
	cvCircle(frame, hand->lastPoint, hand->radius, cvScalarAll(150), 3);
}

void DrawingModule::drawTrajectoryOnFrame(Blob * hand, IplImage * frame){
	//cvZero(frame);

	if(hand->allXYZ.empty())
		return;

	cvSubS(frame, cvScalarAll(15), frame, frame);
	//int i_beg = size >= 20 ? 19 : size-1;

	//for(int i = i_beg; i >= 0; --i){
		
		CvPoint3D32f point = hand->allXYZ[0];
		if(point.x <= 0){
			return;
		}
/*
		int red, green;
		int middle = 128 - point.z;

		if(middle <= 0){
			green = 255 - 2*abs(middle);
			red = 255 - abs(middle);
		}else{
			red = 255 - 2*abs(middle);
			green =  255 - abs(middle);
		}
*/
		cvCircle(frame, cvPoint(point.x/2, point.y/2), 2, cvScalarAll(255), 2);
	//}
}