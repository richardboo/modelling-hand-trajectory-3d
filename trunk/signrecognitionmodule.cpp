#include "signrecognitionmodule.h"
#include "utils.hpp"
#include "image.hpp"

#include <math.h>
#include <QDebug>

using namespace std;

SignRecognitionModule::SignRecognitionModule(const char * fileName){
	load(fileName);
}

SignRecognitionModule::~SignRecognitionModule(void){
}

void SignRecognitionModule::load(const char * fileName){

	CvFileStorage * fs = cvOpenFileStorage(fileName, 0, CV_STORAGE_READ);
	if(fs == NULL)
		return;

	vector<CvMat *> features;

	cvReadIntByName(fs, 0, "id");
	cvReadStringByName(fs, 0, "name");

	int size = cvReadIntByName(fs, 0, "size");

	for(int i = 0; i < size; ++i){
		string name = "feats";
		name.append(Utils::int2string(i));

		CvMat * feat = (CvMat *)cvReadByName(fs, 0, name.c_str(), 0);
		features.push_back(feat);
	}

	cvReleaseFileStorage(&fs);

	// obliczenie sredniej wartosci
	CvMat ** vects = new CvMat*[size];

	// get every vector from database
	for(int j = 0; j < size; ++j){
		vects[j] = features[j];
	}

	CvMat * cov = cvCreateMat(vects[0]->cols, vects[0]->cols, CV_64FC1);
	
	invCov = cvCreateMat(vects[0]->cols,  vects[0]->cols, CV_64FC1);

	// mean vector, just one row
	meanVect = cvCreateMat(1, vects[0]->cols, CV_64FC1);

	cvCalcCovarMatrix((const CvArr**)vects, size, cov, meanVect, CV_COVAR_NORMAL|CV_COVAR_SCALE);
	cvInvert(cov, invCov, CV_SVD);

	cvReleaseMat(&cov);
	delete [] vects;
}

// MahalanobisMean
bool SignRecognitionModule::isSign(IplImage * bwImage, CvRect & rect){

	CvMat * features = cvCreateMat(1, 6, CV_64FC1);

	cvSetImageROI(bwImage, rect);
	
	// here compute features
	CvMoments moments;
	cvMoments(bwImage, &moments, 1);

	FMatrix featsMat(features);

	double s = cvCountNonZero(bwImage);
	IplImage * contour = cvCreateImage(cvGetSize(bwImage), 32, 1);
	cvLaplace(bwImage, contour, 1);
	cvThreshold(contour, contour, 100, 255, CV_THRESH_BINARY);

	double l = (double)cvCountNonZero(contour);
	l = ( l/(double)2*3.1415926f*sqrt(s)-1 );

	featsMat[0][0] = l;
	featsMat[0][1] = cvGetNormalizedCentralMoment(&moments, 2, 0);
	featsMat[0][2] = cvGetNormalizedCentralMoment(&moments, 0, 2);
	featsMat[0][3] = cvGetNormalizedCentralMoment(&moments, 2, 1);
	featsMat[0][4] = cvGetNormalizedCentralMoment(&moments, 1, 2);
	featsMat[0][5] = ((double)rect.height/(double)rect.width);

	double diff = cvMahalanobis((const CvArr*)features, (const CvArr*)meanVect, (const CvArr*)invCov);

	//cvShowImage("reka", bwImage);
	// qDebug() << diff;

	cvReleaseMat(&features);
	cvReleaseImage(&contour);
	cvResetImageROI(bwImage);

	//return (diff <= 8.0);
	return false;
}