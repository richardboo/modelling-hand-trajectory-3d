#include "handdetector.hpp"
#include "image.hpp"
#include "settings.h"

#include <highgui.h>

HandDetector::HandDetector(){
	none = cvScalarAll(0);
}

HandDetector::~HandDetector(){

}


void HandDetector::init(CvSize imgSize){
	allBlobs = cvCreateImage(Settings::instance()->defSize, 8, 1);
	storage = cvCreateMemStorage(0);
}

bool HandDetector::findHand(IplImage * skin, IplImage * blobImg, CvRect & rect, Blob & hand){

	cvZero(blobImg);
	cvCopyImage(skin, allBlobs);

	//cvSmooth(allBlobs, allBlobs, CV_MEDIAN);

	cvSubS(allBlobs, cvScalarAll(5), allBlobs);
	BwImage bw_image(allBlobs);

	bool foundFirst = false;
	for(int i = rect.y; i < rect.y+rect.height; i+=10){
		for(int j = rect.x; j < rect.x+rect.width; j+=10){
			
			if(bw_image[i][j] != 250)
				continue;
			
			cvFloodFill(allBlobs, cvPoint(j, i), cvScalarAll(10), none, none, &component);

			if(component.area < hand.minArea ||
				component.area > hand.maxArea)
				continue;

			if(!foundFirst){
				// pierwszy duzy element
				found = component;
				foundFirst = true;
				cvFloodFill(allBlobs, cvPoint(j, i), cvScalarAll(255), none, none, &component);
				continue;
			}

			// jesli jest juz wiekszy
			if(found.area > 2*component.area)
				continue;

			// jesli mamy element bardziej "na lewo"
			if(found.rect.x < component.rect.x)
				continue;

			// jesli mamy element "nizej"
			if(found.rect.y < component.rect.y)
				continue;

			found = component;
			cvFloodFill(allBlobs, cvPoint(j, i), cvScalarAll(100), none, none, &component);
		}
	}

	// nie znalezlismy zadnego
	if(!foundFirst){
		hand.lastRect = cvRect(-1,-1,-1,-1);
		return false;
	}

	// znalezlismy - musimy go wyluskac
	hand.lastRect = found.rect;
	cvSetImageROI(allBlobs, found.rect);
	cvSetImageROI(blobImg, found.rect);

	cvThreshold(allBlobs, blobImg, 99, 0, CV_THRESH_TOZERO);
	cvThreshold(blobImg, blobImg, 99, 255, CV_THRESH_BINARY);

	cvDilate(blobImg, blobImg, NULL, 1);
	cvErode(blobImg, blobImg, NULL, 1);

/*
	cvClearMemStorage(storage);
	CvSeq* contour = 0;

	cvFindContours(
		blobImg,
		storage,
		&contour,
		sizeof(CvContour),
		CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
	
	for( ; contour != 0; contour = contour->h_next ){
		if(cvContourArea(contour) < 500)
			continue;
		cvDrawContours( blobImg, contour, cvScalarAll(255), cvScalarAll(255), -1, CV_FILLED, 8 );
		break;
	}
*/
	cvResetImageROI(allBlobs);
	cvZero(allBlobs);
	cvSetImageROI(allBlobs, found.rect);

	cvCopyImage(blobImg, allBlobs);

	int all = cvCountNonZero(allBlobs);
	int erode = 0;
	while(all > 70){
		cvErode(allBlobs, allBlobs, 0, 2);
		all = cvCountNonZero(allBlobs);
		erode+=2;
	}

	erode += all/4;

	cvResetImageROI(blobImg);
	cvResetImageROI(allBlobs);

	cvMoments(allBlobs, &moments);
	int x = moments.m10/moments.m00;
	int y = moments.m01/moments.m00;

	hand.lastPoint = cvPoint(x, y);
	hand.radius = erode;

	return true;
}