#pragma once

#include <cv.h>

/**
 * Narzedzia do tworzenia obrazow o okreslonych cechach.
 */
namespace ImageUtils{


	IplImage * getGray(IplImage * frame);
	void getGray(IplImage * frame, IplImage * gray);

	IplImage * createGray(CvSize size = cvSize(640, 480));
	IplImage * create(CvSize size = cvSize(640, 480));
};
