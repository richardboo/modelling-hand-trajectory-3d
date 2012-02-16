#include "calibrationmodule.h"
#include "settings.h"
#include "imageutils.hpp"

#include <QDebug>

//prapares point with (x,y,d) coordinates 
typedef struct elem3_ { 
	float f1; float f2; float f3; 
} elem3; 

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
	int squareSize = 2.1f;

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
			//cvWaitKey(500);
		}
		
	}

	cvReleaseImage(&smaller[0]);
	cvReleaseImage(&smaller[1]);

	cv::displayOverlay("kamera 1", "Przeprowadzam kalibracje...", 3000);
	cv::displayOverlay("kamera 2", "Przeprowadzam kalibracje...", 3000);
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
                objectPoints[k*cornersY*cornersX + i*cornersX + j] = cvPoint3D32f(i*squareSize, j*squareSize, 0);


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
    double rms = cvStereoCalibrate( &_objectPoints, &_imagePoints1,
        &_imagePoints2, &_npoints,
        &_M1, &_D1, &_M2, &_D2,
        imageSize, &_R, &_T, &_E, &_F,
        cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5),
        CV_CALIB_FIX_ASPECT_RATIO + CV_CALIB_ZERO_TANGENT_DIST + CV_CALIB_SAME_FOCAL_LENGTH
    );

	qDebug() << "ERROR: " << rms;

	mQ = cvCreateMat( 4, 4, CV_32F);
	cvZero(mQ);

	cvUndistortPoints( &_imagePoints1, &_imagePoints1,&_M1, &_D1, 0, &_M1 );
    cvUndistortPoints( &_imagePoints2, &_imagePoints2,&_M2, &_D2, 0, &_M2 );


	double R1[3][3], R2[3][3], P1[3][4], P2[3][4];
	CvMat _R1 = cvMat(3, 3, CV_64F, R1);
	CvMat _R2 = cvMat(3, 3, CV_64F, R2);
	CvMat _P1 = cvMat(3, 4, CV_64F, P1);
	CvMat _P2 = cvMat(3, 4, CV_64F, P2);

	cvStereoRectify( &_M1, &_M2, &_D1, &_D2, imageSize,
					&_R, &_T,
					&_R1, &_R2, &_P1, &_P2, mQ,
					0/*CV_CALIB_ZERO_DISPARITY*/ );

/*
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
*/

    //Precompute map for cvRemap()
    cvReleaseMat(&mx[0]);
    cvReleaseMat(&my[0]);
    cvReleaseMat(&mx[1]);
    cvReleaseMat(&my[1]);
    mx[0] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    my[0] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    mx[1] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );
    my[1] = cvCreateMat( imageSize.height,imageSize.width, CV_32F );

	cvInitUndistortRectifyMap(&_M1,&_D1,&_R1,&_P1,mx[0],my[0]);
	cvInitUndistortRectifyMap(&_M2,&_D2,&_R2,&_P2,mx[1],my[1]);

	/*
    cvInitUndistortRectifyMap(&_M1,&_D1,&_R1,&_M1,mx[0],my[0]);
    cvInitUndistortRectifyMap(&_M2,&_D2,&_R2,&_M2,mx[1],my[1]);

	//gets parameters for reprojection matrix Q 
	
	float Fx_= cvmGet( &_M1, 0, 0 ); // focal length in x direction of the rectified image in pixels 
	float Fy_= cvmGet( &_M2, 1, 1 ); // focal length in y direction of the rectified image in pixels 
	float Tx_= cvmGet( &_T, 0, 0 ); // Translation in x direction from the left camera to the right camera 
	float Clx_= cvmGet( &_M1, 0, 2 ); // x coordinate of the optical center of the left camera 
	float Crx_= cvmGet( &_M2, 0, 2 ); // x coordinate of the optical center of the right camera 
	float Cy_= cvmGet( &_M1, 1, 2 ); // y coordinate of the optical center of both left and right cameras 
	
	float Qq[]= { 1, 0, 0, -Clx_, 0, 1, 0, -Cy_, 0, 0, 0, (Fx_+Fy_)/2, 0, 0, -1/Tx_, (Crx_-Clx_)/Tx_ }; 
	mQ = cvCreateMat( 4, 4, CV_32F);
	//cvZero(mQ);
	CvMat mat =	cvMat( 4, 4, CV_32F, Qq );
	cvCopy(&mat, mQ);
	*/

	qDebug() << "matrix" ;
	qDebug() << "|" << cvmGet(mQ, 0, 0) << cvmGet(mQ, 0, 1) << cvmGet(mQ, 0, 2) << cvmGet(mQ, 0, 3) << "|" ;
	qDebug() << "|" << cvmGet(mQ, 1, 0) << cvmGet(mQ, 1, 1) << cvmGet(mQ, 1, 2) << cvmGet(mQ, 1, 3) << "|" ;
	qDebug() << "|" << cvmGet(mQ, 2, 0) << cvmGet(mQ, 2, 1) << cvmGet(mQ, 2, 2) << cvmGet(mQ, 2, 3) << "|" ;
	qDebug() << "|" << cvmGet(mQ, 3, 0) << cvmGet(mQ, 3, 1) << cvmGet(mQ, 3, 2) << cvmGet(mQ, 3, 3) << "|" ;

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

	emit calibrationEnded();

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

	// zapisanie Q
	if(!fwrite(mQ->data.fl,sizeof(float),mQ->rows*mQ->cols,f)) return RESULT_FAIL;

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

	mQ = cvCreateMat( 4, 4, CV_32F );

    FILE* f =  fopen(filename,"rb");
    if(!f) return RESULT_FAIL;
	if(!fread(mx[0]->data.fl,sizeof(float),mx[0]->rows*mx[0]->cols,f)) return RESULT_FAIL;
    if(!fread(my[0]->data.fl,sizeof(float),my[0]->rows*my[0]->cols,f)) return RESULT_FAIL;
    if(!fread(mx[1]->data.fl,sizeof(float),mx[1]->rows*mx[1]->cols,f)) return RESULT_FAIL;
    if(!fread(my[1]->data.fl,sizeof(float),my[1]->rows*my[1]->cols,f)) return RESULT_FAIL;

	// odczytanie q
	if(!fread(mQ->data.fl,sizeof(float),mQ->rows*mQ->cols,f)) return RESULT_FAIL;

	qDebug() << "matrix" ;
	qDebug() << "|" << cvmGet(mQ, 0, 0) << cvmGet(mQ, 0, 1) << cvmGet(mQ, 0, 2) << cvmGet(mQ, 0, 3) << "|" ;
	qDebug() << "|" << cvmGet(mQ, 1, 0) << cvmGet(mQ, 1, 1) << cvmGet(mQ, 1, 2) << cvmGet(mQ, 1, 3) << "|" ;
	qDebug() << "|" << cvmGet(mQ, 2, 0) << cvmGet(mQ, 2, 1) << cvmGet(mQ, 2, 2) << cvmGet(mQ, 2, 3) << "|" ;
	qDebug() << "|" << cvmGet(mQ, 3, 0) << cvmGet(mQ, 3, 1) << cvmGet(mQ, 3, 2) << cvmGet(mQ, 3, 3) << "|" ;

	qDebug() << "sprawdzenie x";

	for(int i = 0; i < 640; ++i){
		
	}


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

void CalibrationModule::setRealCoordinates(Blob * hands[2]){
	
	float x = hands[0]->lastPoint.x * cvmGet(mQ, 0, 0) + cvmGet(mQ, 0, 3);
	float y = hands[0]->lastPoint.y * cvmGet(mQ, 1, 1) + cvmGet(mQ, 1, 3);
	float d = hands[0]->lastDisp/16.0f;
	float z = cvmGet(mQ, 2, 3);
	float w = d * cvmGet(mQ, 3, 2) + cvmGet(mQ, 3, 3);

	x = x/w;
	y = y/w;
	z = z/w;

	hands[0]->lastDisp = z;

	/*

	qDebug() << "real: " << x << y << z;

	float* p=new float[3];
	p[0]=hands[0]->lastPoint.x;
	p[1]=hands[0]->lastPoint.y;
	p[2]=hands[0]->lastDisp;

	cv::Mat _p(1,1,CV_32FC3,p);
	cv::Mat _q(1,1,CV_32FC3);
	
	//perspective tx
	cv::perspectiveTransform(_p,_q,cv::Mat(mQ));
	float x_new=_q.at<cv::Vec3f>(0,0)[0];
	float y_new=_q.at<cv::Vec3f>(0,0)[1];
	float d_new=_q.at<cv::Vec3f>(0,0)[2]; 

	qDebug() << "pix: " << hands[0]->lastPoint.x << hands[0]->lastPoint.y << hands[0]->lastDisp;
	qDebug() << "real: " << x_new << y_new << d_new;

	delete p;
	*/
}