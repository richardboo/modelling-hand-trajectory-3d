#include "stereomodule.h"
#include "image.hpp"
#include "imageutils.hpp"
#include "settings.h"
#include "opencv/matcher.h"


cv::StereoSGBM StereoModule::sgbm;
StereoVar StereoModule::vars;
CvStereoBMState StereoModule::BMState;
CvStereoBMState StereoModule::BMStateCuda;
MyHandBM StereoModule::myHandBMState;
FastStereoState StereoModule::fastState;


StereoModule::StereoModule(void){
	detector = NULL;
	extractor = NULL;
	matcher_popcount = NULL;
	handGray[0] = handGray[1] = NULL;
}

StereoModule::~StereoModule(void){
	if(detector != NULL)
		delete detector;
	if(extractor != NULL)
		delete extractor;
	if(matcher_popcount != NULL)
		delete matcher_popcount;
	if(handGray[0] != NULL)
		cvReleaseImage(&handGray[0]);
	if(handGray[1] != NULL)
		cvReleaseImage(&handGray[1]);

}

void StereoModule::initStates(){
	BMState.preFilterSize=7;
    BMState.preFilterCap=21;
    BMState.SADWindowSize=21;
    BMState.minDisparity=0;
    BMState.numberOfDisparities=64;
    BMState.textureThreshold=1;
    BMState.uniquenessRatio=1;

	BMStateCuda.preFilterSize=7;
    BMStateCuda.preFilterCap=21;
    BMStateCuda.SADWindowSize=21;
    BMStateCuda.minDisparity=0;
    BMStateCuda.numberOfDisparities=64;
    BMStateCuda.textureThreshold=1;
    BMStateCuda.uniquenessRatio=1;

	sgbm.preFilterCap = 31;
    sgbm.SADWindowSize = 11;
    sgbm.P1 = 2*8*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.P2 = 2*32*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.minDisparity = 0;
    sgbm.numberOfDisparities = 32;
    sgbm.uniquenessRatio = 1;
    sgbm.speckleWindowSize = 400;
    sgbm.speckleRange = 16;
    sgbm.disp12MaxDiff = 2;

	vars.levels = 6;
	vars.pyrScale = 0.6;
	vars.nIt = 3;
	vars.minDisp = -32;
	vars.maxDisp = 0;
	vars.poly_n = 3;
	vars.poly_sigma = 0.0;
	vars.fi = 5.0f;
	vars.lambda = 0.1;
	vars.penalization = vars.PENALIZATION_TICHONOV;
	vars.cycle = vars.CYCLE_V;
	vars.flags = vars.USE_SMART_ID | vars.USE_INITIAL_DISPARITY | 1 * vars.USE_MEDIAN_FILTERING ;
}


void StereoModule::stereoStart(CvSize & size){
	
	BMState.roi1 = cvRect(0, 0, size.width, size.height);
	BMState.roi2 = cvRect(0, 0, size.width, size.height);

	BMStateCuda.roi1 = cvRect(0, 0, size.width, size.height);
	BMStateCuda.roi2 = cvRect(0, 0, size.width, size.height);

	sgbm.P1 = 2*8*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.P2 = 2*32*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.fullDP = false;

	if(detector != NULL)
		delete detector;
	if(extractor != NULL)
		delete extractor;
	if(matcher_popcount != NULL)
		delete matcher_popcount;

	detector = new FastFeatureDetector(fastState.featuresTheshold);
	extractor = new BriefDescriptorExtractor(fastState.featuresNr);

	matcher_popcount= new BruteForceMatcher<Hamming>;

	disparityNotNormalized = cvCreateImage(size, IPL_DEPTH_16S, 1);
	disparityNNmat = Mat(Size(size), CV_16S);
	andImage = cvCreateImage(size, 8, 1);

	onlyHandNormalized[0] = cvCreateImage(size, 8, 1);
	onlyHandNormalized[1] = cvCreateImage(size, 8, 1);

	if(handGray[0] == NULL){
		handGray[0] = cvCreateImage(Settings::instance()->defSize, 8, 1);
		handGray[1] = cvCreateImage(Settings::instance()->defSize, 8, 1);
	}
	cvZero(handGray[0]);
	cvZero(handGray[1]);

	//disp = Mat(Size(size), CV_8U);

//GPU
	/*
	d_disp = gpu::GpuMat(Size(size), CV_8U);

	d_remapped1 = gpu::GpuMat(Size(size), CV_8UC3); 
	d_remapped2 = gpu::GpuMat(Size(size), CV_8UC3); 
	*/
}


int StereoModule::stereoProcessGray(IplImage* rectifiedGray[2], IplImage * blobs[2], Blob * hands[2], IplImage * disparity, int type){

	if(type == BMCUDA_){
		/*
		d_left = rectifiedGray[0];
		d_right = rectifiedGray[1];
		bm(d_left, d_right, d_disp);
		disp = d_disp;
		andImage = &IplImage(disp);
		*/
		Mat d;
		vars(Mat(rectifiedGray[1]), Mat(rectifiedGray[0]), d);
		d.convertTo(disp, CV_8U);
		andImage = &IplImage(disp);
		CvScalar s = cvAvg(disparityNotNormalized, blobs[1]);
		cvZero(disparity);
		cvCopy(andImage, disparity);
		hands[0]->setLastPointWithZ(s.val[0]);
		hands[1]->setLastPointWithZ(s.val[0]);
		
		return RESULT_OK;
	}
	else if(type == BM_){
		BMState.roi1 = hands[0]->lastRect;
		BMState.roi2 = hands[1]->lastRect;
		//qDebug() << BMState.preFilterCap;
		//rectifiedGray[0] = cvLoadImage("scene1.row3.col3.ppm", CV_LOAD_IMAGE_GRAYSCALE);
		//rectifiedGray[1] = cvLoadImage("scene1.row3.col5.ppm", CV_LOAD_IMAGE_GRAYSCALE);

		cvZero(disparityNotNormalized);
		cvFindStereoCorrespondenceBM( rectifiedGray[1], rectifiedGray[0], disparityNotNormalized, &BMState);
		cvSmooth(disparityNotNormalized, disparityNotNormalized, CV_MEDIAN, 5);
		//cvCopyImage(disparityNotNormalized, andImage);
		cvNormalize( disparityNotNormalized, andImage, 0, 256, CV_MINMAX );
		//cvCopy(andImage, disparity);

		
		CvScalar s = cvAvg(disparityNotNormalized, blobs[1]);
		cvZero(disparity);
		cvCopy(andImage, disparity, blobs[1]);
		hands[0]->setLastPointWithZ(s.val[0]/16.0f);
		hands[1]->setLastPointWithZ(s.val[0]/16.0f);
		
		return RESULT_OK;
	}
	else if(type == SGBM_){
		
		sgbm(Mat(rectifiedGray[1]), Mat(rectifiedGray[0]), disparityNNmat);
		medianBlur(disparityNNmat, disparityNNmat, 5);

		disparityNNmat.convertTo(disp, CV_8U, 255/(sgbm.numberOfDisparities*16.));
		
		
		CvScalar s = cvAvg(&IplImage(disparityNNmat), blobs[1]);
		
		andImage = &IplImage(disp);
		cvZero(disparity);
		cvCopy(andImage, disparity, blobs[1]);
		hands[0]->setLastPointWithZ(s.val[0]/16.0f);
		hands[1]->setLastPointWithZ(s.val[0]/16.0f);
		
		return RESULT_OK;
	}

	
	return RESULT_OK;

	cvZero(disparity);

	//IplImage * temp = cvCreateImage(Settings::instance()->defSize, 8, 1);
	//cvZero(temp);

	cvSetImageROI(disparity, hands[0]->lastRect);
	cvSetImageROI(blobs[0], hands[0]->lastRect);
	cvSetImageROI(andImage, hands[0]->lastRect);
	//cvSetImageROI(temp, hands[0]->lastRect);

	//cvThreshold(andImage, temp, 1, 255, CV_THRESH_BINARY);
	

	CvScalar avg = cvAvg(andImage, blobs[0]);
	int diff = avg.val[0];
	/*if(diff > 255)	diff = 255;

	if(hands[0]->lastZ != -1){
		diff = (3*diff + 7*hands[0]->lastZ)/10;
	}*/
	avg = cvScalarAll(diff);
	
	cvSet(disparity, avg, blobs[0]);

	//cvResetImageROI(temp);
	cvResetImageROI(blobs[0]);
	cvResetImageROI(andImage);
	cvResetImageROI(disparity);

	//imshow("d", disparity);
	//cvCopyImage(andImage, disparity);

	hands[0]->setLastPointWithZ(avg.val[0]);
	hands[1]->setLastPointWithZ(avg.val[0]);
	

    return RESULT_OK;
}


void StereoModule::stereoProcessMine(IplImage* rectifiedGray[2], IplImage * blobs[2], Blob * hands[2], IplImage * disparity, int type){

	cvZero(disparity);

	if(type == MINE_){

		// proba "dopasowania" jak najlepiej
		CvSize sizeAll = cvSize(min(hands[0]->lastRect.width, hands[1]->lastRect.width),
								min(hands[0]->lastRect.height, hands[1]->lastRect.height));

		cvSetImageROI(andImage, cvRect(0, 0, sizeAll.width, sizeAll.height));
		
		int nonZeroCount = 0;
		int xplus, currMax;
		currMax = 0;
		
		for(int i = -5; i < 6; ++i){

			if(hands[0]->lastRect.x+i < 0 || hands[0]->lastRect.x+sizeAll.width >= Settings::instance()->defSize.width)
				continue;

			cvSetImageROI(blobs[0], cvRect(hands[0]->lastRect.x+i, hands[0]->lastRect.y, sizeAll.width, sizeAll.height));
			cvSetImageROI(blobs[1], cvRect(hands[1]->lastRect.x, hands[1]->lastRect.y, sizeAll.width, sizeAll.height));

			cvAnd(blobs[0], blobs[1], andImage);
			nonZeroCount = cvCountNonZero(andImage);
			if(nonZeroCount > currMax){
				currMax = nonZeroCount;
				xplus = i;
			}

			cvResetImageROI(blobs[0]);
			cvResetImageROI(blobs[1]);
		}

		cvResetImageROI(andImage);
		
		int diff = (hands[1]->lastRect.x+hands[1]->lastRect.width/2 - hands[0]->lastRect.x-hands[0]->lastRect.width/2)+xplus;
		//qDebug() << diff;
		//int diff = abs(hands[0]->lastRect.x+xplus - hands[1]->lastRect.x);

		cvSetImageROI(disparity, hands[0]->lastRect);
		cvSetImageROI(blobs[0], hands[0]->lastRect);

		//diff = diff*255.0f/64.0f;

		cvSet(disparity, cvScalarAll(diff*255.0f/64.0f), blobs[0]);
		cvResetImageROI(blobs[0]);
		cvResetImageROI(disparity);

		hands[0]->setLastPointWithZ(diff);
		hands[1]->setLastPointWithZ(diff);
	}
	else if(type == MINE_BM_){

		BwImage rawDispImage(andImage);
		cvZero(andImage);

		// 1. grey equalize
		CvScalar avg[2];
		for(int i = 0; i < 2; ++i)
			avg[i] = cvAvg(rectifiedGray[i], blobs[i]);

		// roznica koloru
		int diffAvg = avg[0].val[0]-avg[1].val[0];

		//qDebug() << "diff: " << diffAvg;

		// wyrownanie roznic srednich kolorow
		cvAddS(rectifiedGray[1], cvScalar(diffAvg, 0, 0), rectifiedGray[1], blobs[1]);


		cvZero(handGray[0]);
		cvZero(handGray[1]);

		cvCopy(rectifiedGray[0], handGray[0], blobs[0]);
		cvCopy(rectifiedGray[1], handGray[1], blobs[1]);

		BwImage left(handGray[0]);
		BwImage right(handGray[1]);

		int indexToSearch = 1, indexOther = 0;
		int halfDisp = myHandBMState.numberOfDisparities/2;
		int currMin = 300;
		int currVal = 0;
		int realDiffBetween = hands[1]->lastRect.x-hands[0]->lastRect.x;


		// po calej wysokosci obrazka
		for(int y = hands[indexToSearch]->lastRect.y; 
				y < hands[indexToSearch]->lastRect.y + hands[indexToSearch]->lastRect.height; ++y){
			
			// po calej szerokosci obrazka
			for(int x = hands[indexToSearch]->lastRect.x-halfDisp; 
				x < hands[indexToSearch]->lastRect.x + hands[indexToSearch]->lastRect.width-halfDisp; ++x){

				if(left[y][x] == 0)
					continue;

				currMin = 100;
				//int minI = 0;

				// po mozliwych nrOfDisp
				rawDispImage[y][x] = 0;
				for(int i = -halfDisp; (i < halfDisp) && (x+i+realDiffBetween < 640); ++i){
					
					if(right[y][x+i+realDiffBetween] == 0)
						continue;

					currVal = left[y][x+realDiffBetween]-right[y][x+i+realDiffBetween];
					if(currVal < currMin){
						currMin = currVal;
						rawDispImage[y][x+realDiffBetween] = i+realDiffBetween;
						//minI = i;
					}
				}
			}
		}
		/*
		cvAddS(andImage, cvScalar(halfDisp+realDiffBetween,0,0), andImage, blobs[0]);
		*/
		cvSmooth(andImage, andImage, CV_MEDIAN, myHandBMState.medianSmooth);
	
		cvSetImageROI(disparity, hands[0]->lastRect);
		cvSetImageROI(blobs[0], hands[0]->lastRect);
		cvSetImageROI(andImage, hands[0]->lastRect);

		CvScalar avgD = cvAvg(andImage, andImage);
		//cvSet(disparity, avgD, blobs[0]);
		cvCopyImage(andImage, disparity);

		cvResetImageROI(blobs[0]);
		cvResetImageROI(andImage);
		cvResetImageROI(disparity);

		// pokazmy sobie ta mape
		//cvZero(rectifiedGray[0]);
		//cvCopyImage(andImage, rectifiedGray[0]);

		hands[0]->setLastPointWithZ(avgD.val[0]);
		hands[1]->setLastPointWithZ(avgD.val[0]);
		
	}
	else if(type == MINE_RND_){
		
		kpts_1.clear();
		kpts_2.clear();
		mpts_1.clear();
		mpts_2.clear();

		// 1. grey equalize
		CvScalar avg[2];
		for(int i = 0; i < 2; ++i)
			avg[i] = cvAvg(rectifiedGray[i], blobs[i]);

		// roznica koloru
		int diffAvg = avg[0].val[0]-avg[1].val[0];

		//qDebug() << "diff: " << diffAvg;

		// wyrownanie roznic srednich kolorow
		cvAddS(rectifiedGray[1], cvScalar(diffAvg, 0, 0), rectifiedGray[1], blobs[1]);

		// wyselekcjonowanie samego obrazu dloni
		cvZero(handGray[0]);
		cvZero(handGray[1]);
		cvCopy(rectifiedGray[0], handGray[0], blobs[0]);
		cvCopy(rectifiedGray[1], handGray[1], blobs[1]);

		Mat img1(handGray[0]);
		Mat img2(handGray[1]);

		CvSize sizeAll = cvSize(min(hands[0]->lastRect.width, hands[1]->lastRect.width),
								min(hands[0]->lastRect.height, hands[1]->lastRect.height));

		int minW = MIN(hands[0]->lastRect.width, hands[1]->lastRect.width);
		int minH = MIN(hands[0]->lastRect.height, hands[1]->lastRect.height);

		// region dloni
		Mat im1 = img1(cv::Rect(hands[0]->lastRect.x, hands[0]->lastRect.y, minW, minH));
		Mat im2 = img2(cv::Rect(hands[1]->lastRect.x, hands[1]->lastRect.y, minW, minH));

		// znajdujemy keypointsy
		detector->detect(im1, kpts_1);
		detector->detect(im2, kpts_2);

		// ekstracja cech
		Mat desc_1, desc_2;
		extractor->compute(im1, kpts_1, desc_1);
		extractor->compute(im2, kpts_2, desc_2);

		// dopasowanie
		
		matcher_popcount->match(desc_1, desc_2, matches_popcount);
		
		// przygotowanie wektorow punktow dopasowanych
		// sprawdzenie czy leza na mniej wiecej tej samej linii
		// o razu obliczanie disparity miedzy pasujacymi
		int counter = 0;
		int dispAll = 0;
		//mpts_1.reserve(matches_popcount.size());
		//mpts_2.reserve(matches_popcount.size());
		int alldiff = (hands[1]->lastRect.x - hands[0]->lastRect.x);
		for (size_t i = 0; i < matches_popcount.size(); i++){
			const DMatch& match = matches_popcount[i];
			if(abs(kpts_1[match.queryIdx].pt.y - kpts_2[match.trainIdx].pt.y) < 3){
				mpts_1.push_back(kpts_1[match.queryIdx].pt);
				mpts_2.push_back(kpts_2[match.trainIdx].pt);
				counter++;
				int diff = -(kpts_2[match.trainIdx].pt.x - kpts_1[match.queryIdx].pt.x);
				dispAll += diff;
				Point2f p = kpts_2[match.trainIdx].pt;
				cvSetImageROI(disparity, cvRect(	hands[0]->lastRect.x + (p.x-3 > 0 ? p.x-3 : p.x), 
													hands[0]->lastRect.y + (p.y-3 > 0 ? p.y-3 : p.y), 6, 6));
				cvSet(disparity, cvScalarAll(diff+alldiff));
				cvResetImageROI(disparity);
			}
		}

		if(counter == 0){
			return;
		}

		int disp = dispAll/counter;
		disp += alldiff;


		//cvSet(disparity, cvScalarAll(disp*255.0f/64.0f), blobs[0]);
		hands[0]->setLastPointWithZ(disp);
		hands[1]->setLastPointWithZ(disp);

		return;

	}

}
