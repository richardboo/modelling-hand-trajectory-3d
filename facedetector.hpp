#pragma once

#include <cv.h>
#include "blob.hpp"

/**
 * Modul rozpoznawania twarzy.
 * Wykorzystuje algorytm Haar'a i dane z pliku haarcascade_frontalface_alt.xml.
 */
class FaceDetector{
public:
	FaceDetector();
	~FaceDetector();
	bool init();

	//Blob head;
	CvRect lastFound;

	bool findHeadHaar(IplImage * frame, Blob * head);

private:
	static CvHaarClassifierCascade	*cascade;
	static CvMemStorage				*storage;

};