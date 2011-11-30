#pragma once

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <iostream>

class SignRecognitionModule
{
public:
	SignRecognitionModule(const char * name);
	~SignRecognitionModule(void);

	void load(const char * fileName);

	bool isSign(IplImage * blob, CvRect & rect);

private:

	CvMat * meanVect;
	CvMat * invCov;
};
