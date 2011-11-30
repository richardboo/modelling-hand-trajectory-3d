#include "stereomodule.h"
#include "image.hpp"
#include "imageutils.hpp"
#include "settings.h"


cv::StereoSGBM StereoModule::sgbm;
CvStereoBMState StereoModule::BMState;
CvStereoBMState StereoModule::BMStateCuda;

StereoModule::StereoModule(void)
{
}

StereoModule::~StereoModule(void)
{
}



void StereoModule::stereoStart(CvSize & size){
	
	//BMState = cvCreateStereoBMState();
    /*
	BMState.preFilterSize=7;
    BMState.preFilterCap=21;
    BMState.SADWindowSize=21;
    BMState.minDisparity=0;
    BMState.numberOfDisparities=64;
    BMState.textureThreshold=1;
    BMState.uniquenessRatio=1;
	*/
	BMState.roi1 = cvRect(0, 0, size.width, size.height);
	BMState.roi2 = cvRect(0, 0, size.width, size.height);

	BMStateCuda.roi1 = cvRect(0, 0, size.width, size.height);
	BMStateCuda.roi2 = cvRect(0, 0, size.width, size.height);

	/*
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
	*/
	sgbm.P1 = 2*8*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.P2 = 2*32*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.fullDP = false;

	disparityNotNormalized = cvCreateImage(size, IPL_DEPTH_16S, 1);
	disparityNNmat = Mat(Size(size), CV_16S);
	andImage = cvCreateImage(size, 8, 1);

	onlyHandNormalized[0] = cvCreateImage(size, 8, 1);
	onlyHandNormalized[1] = cvCreateImage(size, 8, 1);

	disp = Mat(Size(size), CV_8U);
/*
GPU
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
	}
	else if(type == BM_){
		cvFindStereoCorrespondenceBM( rectifiedGray[0], rectifiedGray[1], disparityNotNormalized, &BMState);
		cvNormalize( disparityNotNormalized, andImage, 0, 256, CV_MINMAX );
	}
	else if(type == SGBM_){
		sgbm(Mat(rectifiedGray[0]), Mat(rectifiedGray[1]), disparityNNmat);
		disparityNNmat.convertTo(disp, CV_8U, 255/(sgbm.numberOfDisparities*16.));
		andImage = &IplImage(disp);
	}

	cvZero(disparity);

	cvSetImageROI(disparity, hands[0]->lastRect);
	cvSetImageROI(blobs[0], hands[0]->lastRect);
	cvSetImageROI(andImage, hands[0]->lastRect);

	CvScalar avg = cvAvg(andImage, blobs[0]);
	int diff = avg.val[0]*4;
	if(diff > 255)	diff = 255;

	if(hands[0]->lastZ != -1){
		diff = (3*diff + 7*hands[0]->lastZ)/10;
	}
	avg = cvScalarAll(diff);
	
	cvSet(disparity, avg, blobs[0]);

	cvResetImageROI(blobs[0]);
	cvResetImageROI(andImage);
	cvResetImageROI(disparity);

	//imshow("d", disparity);

	hands[0]->setLastPointWithZ(avg.val[0]);
	hands[1]->setLastPointWithZ(avg.val[0]);
	
    return RESULT_OK;
}


void StereoModule::stereoProcessMine(IplImage* rectifiedGray[2], IplImage * blobs[2], Blob * hands[2], IplImage * disparity, int type){


	if(type == MINE_){

		// proba "dopasowania" jak najlepiej
		CvSize sizeAll = cvSize(min(hands[0]->lastRect.width, hands[1]->lastRect.width),
								min(hands[0]->lastRect.height, hands[1]->lastRect.height));

		cvSetImageROI(andImage, cvRect(0, 0, sizeAll.width, sizeAll.height));
		
		int nonZeroCount = 0;
		int xplus, currMax;
		currMax = 0;
		for(int i = -5; i < 6; ++i){

			if(hands[0]->lastRect.x+i < 0 || hands[0]->lastRect.x+sizeAll.width >= Settings::instance()->imageSize.width)
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
		

		int diff = hands[0]->lastRect.x+xplus - hands[1]->lastRect.x + 64;
	
		// 128 - najwieksze disparity
		diff *= 2;
		if(diff > 255)	diff = 255;

		if(hands[0]->lastZ != -1){
			diff = (3*diff + 7*hands[0]->lastZ)/10;
		}

		cvZero(disparity);

		cvSetImageROI(disparity, hands[0]->lastRect);
		cvSetImageROI(blobs[0], hands[0]->lastRect);
		cvSet(disparity, cvScalarAll(diff), blobs[0]);
		cvResetImageROI(blobs[0]);
		cvResetImageROI(disparity);

		hands[0]->setLastPointWithZ(diff);
		hands[1]->setLastPointWithZ(diff);
	}
	else if(type == MINE_BM_){
	
		cvZero(andImage);

		int indexToSearch = 0, indexOther = 1;
		int diff = (hands[0]->lastRect.x - hands[1]->lastRect.x);

		BwImage bw0(rectifiedGray[indexToSearch]);
		BwImage bw1(rectifiedGray[indexOther]);
		BwImage disp(andImage);

		for(int y = hands[indexToSearch]->lastRect.y; 
				y < hands[indexToSearch]->lastRect.y + hands[indexToSearch]->lastRect.height; ++y){
			
					int nextI = 0;
			for(int x = hands[indexToSearch]->lastRect.x; 
				x < hands[indexToSearch]->lastRect.x + hands[indexToSearch]->lastRect.width; ++x){
				
				// szukamy tylko pierwszych 5
				int currMin = 300;
				int diff = 0;
				int xfound = -1;
				int beginX = hands[indexOther]->lastRect.x+nextI;
				for(int i = beginX; i < beginX+5 && i < Settings::instance()->imageSize.width; ++i){
					
					if(bw0[y][x] == 0 || bw1[y][i] == 0){
						continue;
					}

					diff = abs(bw0[y][x] - bw1[y][i]);
					if(diff < currMin && diff < 5){
						currMin = diff;
						xfound = i;
					}
				}
				if(xfound != -1){
					disp[y][x] = diff+xfound-beginX+64;
					nextI = xfound-beginX;
				}else{
					disp[y][x] = diff+64;
					nextI++;
				}
			}
		}

		CvScalar avg = cvAvg(andImage, blobs[indexToSearch]);
		diff = avg.val[0];
		diff *= 2;
		if(diff > 255)	diff = 255;

		if(hands[0]->lastZ != -1){
			diff = (3*diff + 7*hands[0]->lastZ)/10;
		}

		hands[0]->setLastPointWithZ(diff);
		hands[1]->setLastPointWithZ(diff);

		cvZero(disparity);
		cvSet(disparity, cvScalarAll(diff), blobs[indexToSearch]);

	}
	else if(type == MINE_RND_){
		
	}

}
