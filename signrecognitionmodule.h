#pragma once

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <iostream>

/**
 * Modul prostego rozpoznawania znaku pokazywanego dlonia.
 * Rozpoznaje znak jezyka migowego "A",
 * cechy znaku zawarte sa w pliku A.xml.
 */
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
