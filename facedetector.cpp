#include "facedetector.hpp"

#include "settings.hpp"


using namespace std;

CvHaarClassifierCascade	* FaceDetector::cascade = NULL;
CvMemStorage			* FaceDetector::storage = NULL;

FaceDetector::FaceDetector(){
	lastFound = cvRect(-1,-1,-1,-1);
}

FaceDetector::~FaceDetector(){

}

bool FaceDetector::init(){
	if(!cascade)
		cascade = (CvHaarClassifierCascade *)cvLoad( "haarcascade_frontalface_alt.xml", 0, 0, 0 );
	if(!storage)
		storage = cvCreateMemStorage(0);

	if(!cascade)
		return false;

	return true;
}

bool FaceDetector::findHeadHaar(IplImage * frame){
	CvRect biggerLast = head.getBiggerRect();
	
	if(head.lastRect.height != -1)
		cvSetImageROI(frame, biggerLast);

	// szukanie twarzy przez Haar'a
	CvSeq *faces = cvHaarDetectObjects(
			frame,
			cascade,
			storage,
			1.3,
			3,
			0 /*CV_HAAR_DO_CANNY_PRUNNING*/,
			cvSize( 140, 140 ) );

	CvSize biggest = cvSize(0,0);
	CvRect nextFace = cvRect(-1,-1,-1,-1);

	// wybranie najwiekszej
	for(int i = 0 ; i < ( faces ? faces->total : 0 ) ; i++ ) {

		CvRect *r = ( CvRect* )cvGetSeqElem( faces, i );
		if(r->width > biggest.width && r->height > biggest.height){
			/*if( head.lastRect.height != -1 && 
				abs(r->x - head.lastRect.x) >= Blob::plusRectSize &&
				abs(r->y - head.lastRect.y) >= Blob::plusRectSize)
				continue;
			*/
			nextFace = cvRect(r->x, r->y, r->width, r->height);
			biggest.width = r->width;
			biggest.height = r->height;
		}
	}
	if(head.lastRect.height != -1){
		cvResetImageROI(frame);
	}

	if(nextFace.height == -1){
		head.lastRect.height = -1;
		return false;
	}

	// ok, znaleziono
	if(head.lastRect.height != -1){
		head.lastRect = cvRect(biggerLast.x+nextFace.x, biggerLast.y + nextFace.y,
			nextFace.width, nextFace.height);
		lastFound = cvRect(biggerLast.x+nextFace.x, biggerLast.y + nextFace.y,
			nextFace.width, nextFace.height);
	}else{
		head.lastRect = nextFace;
		lastFound = nextFace;
	}
	
	return true;
}