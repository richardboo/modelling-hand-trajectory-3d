#pragma once

#include <cv.h>
#include <vector>

#include "blob.hpp"


class HandDetector{
public:
	HandDetector();
	~HandDetector();

	void init(CvSize imgSize);

	int findHand(IplImage * skin, IplImage * blobImg,  IplImage * original, CvRect & rect, Blob & hand, Blob & head);

private:
	CvConnectedComp component, found;
	CvScalar none;
	CvMemStorage * storage;
	CvMoments moments;
	
	IplImage * allBlobs;
};