#include "blob.hpp"
#include "utils.hpp"
#include "settings.h"

#include <QDebug>

int Blob::plusRectSize = 50;
int Blob::idCounter = 0;

Blob::Blob(void): id(idCounter){

	lastRect = cvRect(-1,-1,-1,-1);
	lastXYZ = cvPoint3D32f(-1, -1, -1);
	
	lastPoint = cvPoint(-1,-1);
	lastDisp = -1;

	lastKnownRect = cvRect(-1,-1,-1,-1);

	color = cvScalar( rand()&255, rand()&255, rand()&255 );

	minArea = 3000;
	maxArea = 50000;

	mask = cvCreateImage(Settings::instance()->defSize, 8, 1);
	cvZero(mask);

	radius = -1;
	lastZ = -1;

	occluded = false;

	idCounter++;
}

Blob::~Blob(void){
	//cvReleaseImage(&mask);
}

CvRect Blob::getBiggerRect(int plus){
	if(lastRect.height == -1)
		return lastRect;

	int newX = Utils::subMoreThanZero(lastRect.x, plus);
	int newY = Utils::subMoreThanZero(lastRect.y, plus);

	return cvRect(	newX,
					newY,
					Utils::addLessThan(lastRect.width, 2*plus, 640-newX),
					Utils::addLessThan(lastRect.height, 2*plus, 480-newY));
}


CvRect Blob::getSmallerRect(){
	if(lastRect.x == -1)
		return lastRect;

	return cvRect(	lastRect.x + lastRect.width/4,
					lastRect.y + lastRect.height/4,
					lastRect.width/2,
					lastRect.height/2);
}

void Blob::setLastRect(CvRect & rect){
	lastRect = rect;
	//cvCopyImage(maskToCopy, mask);

	/*
	cvZero(mask);
	cvSetImageROI(mask, getBiggerRect());
	cvSet(mask, cvScalarAll(255));
	cvResetImageROI(mask);
	*/
}

void Blob::setLastPointWithZ(int z){

	if(z == -1){
		lastXYZ = cvPoint3D32f(-1, -1, -1);
		lastDisp = -1;
	}else{
		lastXYZ = cvPoint3D32f(lastPoint.x, lastPoint.y, z);
		lastDisp = z;
		//qDebug() << "point: " << lastPoint.x << lastPoint.y << z;
	}
	allXYZ.push_front(lastXYZ);
	lastZ = z;
}

void Blob::setRealPoint(float x, float y, float z){
	
	allXYZReal.push_back(cvPoint3D32f(x, y, z));

}