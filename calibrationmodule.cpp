#include "calibrationmodule.h"
#include "settings.h"
#include "imageutils.hpp"

CalibrationModule::CalibrationModule(void){
	this->imageSize = Settings::instance()->defSize;
    mx[0] = mx[1] = my[0] = my[1] = 0;
    calibrationStarted = false;
    calibrationDone = false;
	imageWithCorners = NULL;
	sampleCount = 0;
	maxSamples = 0;
	
}

CalibrationModule::~CalibrationModule(void){
	if(imageWithCorners != NULL)
		cvReleaseImage(&imageWithCorners);
}


void CalibrationModule::calibrationStart(int cornersX,int cornersY, int max){
    this->cornersX = cornersX;
    this->cornersY = cornersY;
	maxSamples = max;

	boardSize = cvSize(cornersX, cornersY);
    this->cornersN = cornersX*cornersY;
    ponintsTemp[0].resize(cornersN);
    ponintsTemp[1].resize(cornersN);
    sampleCount = 0;
    calibrationStarted = true;

	leftImages.clear();
	rightImages.clear();
}

// add grey samples
int CalibrationModule::calibrationAddSample(IplImage* imageLeft,IplImage* imageRight){

    if(!calibrationStarted) 
		return RESULT_FAIL;

	IplImage * copyLeft = cvCloneImage(imageLeft);
	IplImage * copyRight = cvCloneImage(imageRight);

	leftImages.push_back(copyLeft);
	rightImages.push_back(copyRight);
	sampleCount++;
	return RESULT_OK;
}


int CalibrationModule::calibrationEnd(){
    calibrationStarted = false;

	IplImage * smaller[2];
	smaller[0] = cvCreateImage(Settings::instance()->defSmallSize, 8, 3);
	smaller[1] = cvCreateImage(Settings::instance()->defSmallSize, 8, 3);


    int succeses = 0;
	sampleCount = 0;

	for(int j = 0; j < leftImages.size(); ++j){
	
		succeses = 0;
		IplImage* image[2] = {leftImages[j], rightImages[j]};

		for(int lr=0;lr<2;lr++){
			CvSize imageSize =  cvGetSize(image[lr]);

			if(imageSize.width != this->imageSize.width || imageSize.height != this->imageSize.height)
				return RESULT_FAIL;

			int cornersDetected = 0;

			//FIND CHESSBOARDS AND CORNERS THEREIN:
			int result = cvFindChessboardCorners(
				image[lr], cvSize(cornersX, cornersY),
				&ponintsTemp[lr][0], &cornersDetected,
				CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE
			);

			if(result && cornersDetected == cornersN){

				//Calibration will suffer without subpixel interpolation
				cvFindCornerSubPix(
					image[lr], &ponintsTemp[lr][0], cornersDetected,
					cvSize(11, 11), cvSize(-1,-1),
					cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,30, 0.01)
				);
				succeses++;
			}

		}
		if(succeses == 2){
			for(int lr=0;lr<2;lr++){
				points[lr].resize((sampleCount+1)*cornersN);
				copy( ponintsTemp[lr].begin(), ponintsTemp[lr].end(),  points[lr].begin() + sampleCount*cornersN);
			}
			sampleCount++;

			// odrysowanie
			if(!imageWithCorners){
				imageWithCorners = ImageUtils::create(imageSize);
			}
			cvCvtColor(leftImages[j], imageWithCorners, CV_GRAY2BGR);

			cvDrawChessboardCorners(imageWithCorners, boardSize, &ponintsTemp[0][0], ponintsTemp[0].size(), true);
			cvResize(imageWithCorners, smaller[0]);
			cvShowImage("kamera 1", smaller[0]);
			cv::displayOverlay("kamera 1", "Znaleziona szachownica kamera 1", 1000);
			//cvWaitKey(1000);

			cvCvtColor(rightImages[j], imageWithCorners, CV_GRAY2BGR);
			cvDrawChessboardCorners(imageWithCorners, boardSize, &ponintsTemp[1][0], ponintsTemp[1].size(), true);
			cvResize(imageWithCorners, smaller[1]);
			cvShowImage("kamera 2", smaller[1]);
			cv::displayOverlay("kamera 2", "Znaleziona szachownica kamera 2", 1000);
			cvWaitKey(500);
		}
		
	}

	cvReleaseImage(&smaller[0]);
	cvReleaseImage(&smaller[1]);


	//cvDestroyWindow("corners");

    // ARRAY AND VECTOR STORAGE:
    double M1[3][3], M2[3][3], D1[5], D2[5];
    double R[3][3], T[3], E[3][3], F[3][3];
    CvMat _M1,_M2,_D1,_D2,_R,_T,_E,_F;

    _M1 = cvMat(3, 3, CV_64F, M1 );
    _M2 = cvMat(3, 3, CV_64F, M2 );
    _D1 = cvMat(1, 5, CV_64F, D1 );
    _D2 = cvMat(1, 5, CV_64F, D2 );
    _R = cvMat(3, 3, CV_64F, R );
    _T = cvMat(3, 1, CV_64F, T );
    _E = cvMat(3, 3, CV_64F, E );
    _F = cvMat(3, 3, CV_64F, F );

    // HARVEST CHESSBOARD 3D OBJECT POINT LIST:
    objectPoints.resize(sampleCount*cornersN);

    for(int k=0;k<sampleCount;k++)
        for(int i = 0; i < cornersY; i++ )
            for(int j = 0; j < cornersX; j++ )
                objectPoints[k*cornersY*cornersX + i*cornersX + j] = cvPoint3D32f(i, j, 0);


    npoints.resize(sampleCount,cornersN);

    int N = sampleCount * cornersN;

    CvMat _objectPoints = cvMat(1, N, CV_32FC3, &objectPoints[0] );
    CvMat _imagePoints1 = cvMat(1, N, CV_32FC2, &points[0][0] );
    CvMat _imagePoints2 = cvMat(1, N, CV_32FC2, &points[1][0] );
    CvMat _npoints = cvMat(1, npoints.size(), CV_32S, &npoints[0] );
    cvSetIdentity(&_M1);
    cvSetIdentity(&_M2);
    cvZero(&_D1);
    cvZero(&_D2);

    //CALIBRATE THE STEREO CAMERAS
    cvStereoCalibrate( &_objectPoints, &_imagePoints1,
        &_imagePoints2, &_npoints,
        &_M1, &_D1, &_M2, &_D2,
        imageSize, &_R, &_T, &_E, &_F,
        cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5),
        CV_CALIB_FIX_ASPECT_RATIO + CV_CALIB_ZERO_TANGENT_DIST + CV_CALIB_SAME_FOCAL_LENGTH
    );

    //Always work in undistorted space
    cvUndistortPoints( &_imagePoints1, &_imagePoints1,&_M1, &_D1, 0, &_M1 );
    cvUndistortPoints( &_imagePoints2, &_imagePoints2,&_M2, &_D2, 0, &_M2 );

    //COMPUTE AND DISPLAY RECTIFICATION


    double R1[3][3], R2[3][3];
    CvMat _R1 = cvMat(3, 3, CV_64F, R1);
    CvMat _R2 = cvMat(3, 3, CV_64F, R2);

    //HARTLEY'S RECTIFICATION METHOD
    double H1[3][3], H2[3][3], iM[3][3];
    CvMat _H1 = cvMat(3, 3, CV_64F, H1);
    CvMat _H2 = cvMat(3, 3, CV_64F, H2);
    CvMat _iM = cvMat(3, 3, CV_64F, iM);

    cvStereoRectifyUncalibrated(
        &_imagePoints1,&_imagePoints2, &_F,
        imageSize,
        &_H1, &_H2, 3
    );
    cvInvert(&_M1, &_iM);
    cvMatMul(&_H1, &_M1, &_R1);
    cvMatMul(&_iM, &_R1, &_R1);
    cvInvert(&_M2, &_iM);
    cvMatMul(&_H2, &_M2, &_R2);
    cvMatMul(&_iM, &_R2, &_R2);


    //Precompute map for cvRemap()
    cvReleaseMat(&mx[0]);
    cvReleaseMat(&my[0]);
    cvReleaseMat(&mx[1]);
    cvReleaseMat(&my[1]);
    mx[0] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    my[0] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    mx[1] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    my[1] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );

    cvInitUndistortRectifyMap(&_M1,&_D1,&_R1,&_M1,mx[0],my[0]);
    cvInitUndistortRectifyMap(&_M2,&_D2,&_R2,&_M2,mx[1],my[1]);
/*
GPU
	m_mx1 = mx1;
	m_mx2 = mx2;
	m_my1 = my1;
	m_my2 = my2;

	d_mx1 = m_mx1;
	d_mx2 = m_mx2;
	d_my1 = m_my1;
	d_my2 = m_my2;
*/
    calibrationDone = true;

    return RESULT_OK;
}

void CalibrationModule::rectifyImage(IplImage * frame, IplImage * imagesRectified, int i){
	cvRemap( frame, imagesRectified, mx[i], my[i], CV_INTER_NN+CV_WARP_FILL_OUTLIERS);
}


int CalibrationModule::calibrationSave(const char* filename){
    if(!calibrationDone) return RESULT_FAIL;

    FILE* f =  fopen(filename,"wb");
    if(!f) return RESULT_FAIL;
    if(!fwrite(mx[0]->data.fl,sizeof(float),mx[0]->rows*mx[0]->cols,f)) return RESULT_FAIL;
    if(!fwrite(my[0]->data.fl,sizeof(float),my[0]->rows*my[0]->cols,f)) return RESULT_FAIL;
    if(!fwrite(mx[1]->data.fl,sizeof(float),mx[1]->rows*mx[1]->cols,f)) return RESULT_FAIL;
    if(!fwrite(my[1]->data.fl,sizeof(float),my[1]->rows*my[1]->cols,f)) return RESULT_FAIL;
    fclose(f);
    return RESULT_OK;
}


int CalibrationModule::calibrationLoad(const char* filename){
    cvReleaseMat(&mx[0]);
    cvReleaseMat(&my[0]);
    cvReleaseMat(&mx[1]);
    cvReleaseMat(&my[1]);

    mx[0] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    my[0] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    mx[1] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    my[1] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    FILE* f =  fopen(filename,"rb");
    if(!f) return RESULT_FAIL;
	if(!fread(mx[0]->data.fl,sizeof(float),mx[0]->rows*mx[0]->cols,f)) return RESULT_FAIL;
    if(!fread(my[0]->data.fl,sizeof(float),my[0]->rows*my[0]->cols,f)) return RESULT_FAIL;
    if(!fread(mx[1]->data.fl,sizeof(float),mx[1]->rows*mx[1]->cols,f)) return RESULT_FAIL;
    if(!fread(my[1]->data.fl,sizeof(float),my[1]->rows*my[1]->cols,f)) return RESULT_FAIL;
    fclose(f);
/*
GPU
	m_mx1 = mx1;
	m_mx2 = mx2;
	m_my1 = my1;
	m_my2 = my2;

	d_mx1 = m_mx1;
	d_mx2 = m_mx2;
	d_my1 = m_my1;
	d_my2 = m_my2;
*/
    calibrationDone = true;
    return RESULT_OK;
}