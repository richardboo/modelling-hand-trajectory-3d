#pragma once

#include <cv.h>

#include "blob.hpp"

class HandDetector{
public:
	HandDetector();
	~HandDetector();

	void init(CvSize imgSize);

	Blob hand;
	Blob * head;


	bool findHand(IplImage * skin, IplImage * blobImg, CvRect & rect, Blob & hand);

private:
	CvConnectedComp component, found;
	CvScalar none;
	CvMemStorage * storage;
	CvMoments moments;

	IplImage * allBlobs;
};