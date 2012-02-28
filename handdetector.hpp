#pragma once

#include <cv.h>
#include <vector>

#include "blob.hpp"

/**
 * Modul segmentacji dloni z obrazu z wykrytymi obszarami skory.
 * Wykorzystuje znana poprzednia pozycje dloni oraz pozycje glowy (jesli zostala wykryta).
 */
class HandDetector{
public:
	HandDetector();
	~HandDetector();

	void init(CvSize imgSize);

	int findHand(IplImage * skin, IplImage * blobImg,  IplImage * original, CvRect & rect, Blob & hand, Blob & head);

private:
	CvConnectedComp possibleHand, possibleHead, component;
	CvPoint pointHand, pointHead;

	std::vector<CvConnectedComp> blobsFound;
	std::vector<CvPoint> blobsFoundPoints;

	CvScalar none;
	CvMemStorage * storage;
	CvMoments moments;
	
	IplImage * allBlobs;
};