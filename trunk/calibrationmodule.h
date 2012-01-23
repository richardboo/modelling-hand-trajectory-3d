#pragma once

#include <opencv/cv.h>
#include <highgui.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "blob.hpp"


class CalibrationModule
{
private:
	
	int cornersX,cornersY,cornersN;
    int sampleCount;
    bool calibrationStarted;
    bool calibrationDone;

    CvSize imageSize;
	CvSize boardSize;
    int imageWidth;
    int imageHeight;
	int maxSamples;

    vector<CvPoint2D32f> ponintsTemp[2];
    vector<CvPoint3D32f> objectPoints;
    vector<CvPoint2D32f> points[2];
    vector<int> npoints;

public:
    CalibrationModule(void);
	~CalibrationModule(void);

    //matrices resulting from calibration (used for cvRemap to rectify images)
    //CvMat *mx1,*my1,*mx2,*my2;

	CvMat *mx[2],*my[2];
	CvMat mQ;

	IplImage * imageWithCorners;
	vector<IplImage *> leftImages;
	vector<IplImage *> rightImages;

	//IplImage * r, *g, * b, *black;

    void calibrationStart(int cornersX, int cornersY, int max);
    int calibrationAddSample(IplImage* imageLeft, IplImage* imageRight);
    int calibrationEnd();

    int calibrationSave(const char* filename);
    int calibrationLoad(const char* filename);

	void rectifyImage(IplImage * frame, IplImage * imagesRectified, int i);
	void setRealCoordinates(Blob * hands[2]);

    CvSize getImageSize()		{return imageSize;}
    bool getCalibrationStarted(){return calibrationStarted;}
    bool getCalibrationDone()	{return calibrationDone;}
    int getSampleCount()		{return sampleCount;}
	int getMaxSamples()			{return maxSamples;}

	// dla gpu
	/*
	gpu::GpuMat d_mx1, d_mx2, d_my1, d_my2;
	Mat m_mx1, m_mx2, m_my1, m_my2;
	*/
};
