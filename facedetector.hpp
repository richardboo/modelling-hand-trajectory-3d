#pragma once

#include <cv.h>
#include "blob.hpp"

class FaceDetector{
public:
	FaceDetector();
	~FaceDetector();
	bool init();

	Blob head;
	CvRect lastFound;

	bool findHeadHaar(IplImage * frame);

private:
	static CvHaarClassifierCascade	*cascade;
	static CvMemStorage				*storage;

};