#include "skindetector.hpp"
#include "settings.h"
#include "image.hpp"
#include "utils.hpp"

#include <highgui.h>

SkinDetector::SkinDetector(void){
	skinFound = false;
	maskTemp = cvCreateImage(Settings::instance()->defSize, 8, 1);
	temp = cvCreateImage(Settings::instance()->defSize, 8, 3);

	skinMask = cvCreateImage(Settings::instance()->defSize, 8, 1);

	channel0 = cvCreateImage(Settings::instance()->defSize, 8, 1);
	channel1 = cvCreateImage(Settings::instance()->defSize, 8, 1);
	channel2 = cvCreateImage(Settings::instance()->defSize, 8, 1);

	framesCounter = 0;
}

SkinDetector::~SkinDetector(void)
{
	cvReleaseImage(&maskTemp);
	cvReleaseImage(&temp);
	cvReleaseImage(&skinMask);

	cvReleaseImage(&channel0);
	cvReleaseImage(&channel1);
	cvReleaseImage(&channel2);
}

void SkinDetector::updateHistogram(IplImage * frame, CvRect & rect){

	framesCounter++;

	cvSetImageROI(frame, rect);
	cvSetImageROI(temp, rect);
	cvSetImageROI(maskTemp, rect);
	cvSetImageROI(skinMask, rect);

	CvScalar avg = cvAvg(frame);
	
	cvSetImageROI(channel0, rect);
	cvSetImageROI(channel1, rect);
	cvSetImageROI(channel2, rect);

	// r - channel2
	// g - channel1
	// b - channel0
	cvSplit(frame, channel0, channel1, channel2, NULL);

	// bierzemy tylko te, gdzie r jest max
	// r > g
	cvCmp(channel2, channel1, skinMask, CV_CMP_GT);

	// r > b
	cvCmp(channel2, channel0, maskTemp, CV_CMP_GT);
	cvAnd(skinMask, maskTemp, skinMask);

	// w skinMask jest maska, gdzie r > g i r > b, reszte trzeba ustawic na avg 
	cvZero(temp);
	cvSet(temp, avg);
	cvCopy(frame, temp, skinMask);

	//cvShowImage("skin hist", temp);
	//cvWaitKey(5000);

	cvResetImageROI(channel0);
	cvResetImageROI(channel1);
	cvResetImageROI(channel2);

	cvResetImageROI(frame);
	cvResetImageROI(temp);
	cvResetImageROI(maskTemp);
	cvResetImageROI(skinMask);

	skinHistogram.update(temp, rect);

	// troche jasniejsze
	cvAddS(temp, cvScalarAll(10), temp);
	skinHistogram.update(temp, rect);

	// troche ciemniejsze
	cvSubS(temp, cvScalarAll(10), temp);
	skinHistogram.update(temp, rect);
}

void SkinDetector::detectSkin(IplImage * frame, IplImage * skin, CvRect & rect, int type, IplImage * fg){

	switch(type){
		case RGB_:	detectSkinRGB(frame, skin, rect);	break;
		case HSV_:	detectSkinHSV(frame, skin, rect);	break;
		case HIST_:	detectSkinHist(frame, skin, rect);	break;
		case ALL_:	detectSkinAll(frame, skin, rect);	break;
		case BGSKIN_:	detectSkinBgR(frame, skin, rect, fg);	break;
		case ONLY_BG_:	detectSkinBg(frame, skin, rect, fg);	break;
	}
}


void SkinDetector::detectSkinRGB(IplImage * frame, IplImage * skin, CvRect & rect){
	
	cvZero(skin);
	RgbImage rgb(frame);
	BwImage skin_img(skin);

	RgbPixel pix;
	
	for(int i = rect.y; i < rect.y+rect.height; ++i){
		for(int j = rect.x; j < rect.x+rect.width; ++j){

				pix = rgb[i][j];
				
				if(	pix.r > 95 &&
					pix.g > 40 &&
					pix.b > 20 &&
					(Utils::maximum(pix.r, pix.g, pix.b) - Utils::minimum(pix.r, pix.g, pix.b) 
							> 15) &&
					(abs(pix.r - pix.g) > 15)&&
					(pix.r > pix.b) &&
					(pix.r > pix.g)
					){
					skin_img[i][j] = 255;
			}
		}
	}

	cvDilate(skin, skin);
	//cvDilate(skin, skin);
}

void SkinDetector::detectSkinHSV(IplImage * frame, IplImage * skin, CvRect & rect){

	cvZero(skin);
	cvCvtColor(frame, temp, CV_BGR2HSV);
	HsvImage hsv(temp);
	BwImage skin_img(skin);

	HsvPixel pix;
	
	for(int i = rect.y; i < rect.y+rect.height; ++i){
		for(int j = rect.x; j < rect.x+rect.width; ++j){

				pix = hsv[i][j];
				
				if(	pix.h < 20 &&

					pix.s > 30 &&
					pix.s < 150 &&

					pix.v > 80){
					skin_img[i][j] = 255;
			}
		}
	}
	cvDilate(skin, skin);
	
	//cvDilate(skin, skin);
}

void SkinDetector::detectSkinYCRCB(IplImage * frame, IplImage * skin, CvRect & rect){
	
	cvZero(skin);

	cvCvtColor(frame, temp, CV_BGR2YCrCb);

	YCrCbImage ycrcb(temp);
	BwImage skin_img(skin);

	YCrCbPixel pix;
	
	for(int i = rect.y; i < rect.y+rect.height; ++i){
		for(int j = rect.x; j < rect.x+rect.width; ++j){

				pix = ycrcb[i][j];
				
				if(	pix.cr <= 173 &&
					pix.cr >= 133 &&
					pix.cb >= 77 &&
					pix.cb >= 127 
					){
					skin_img[i][j] = 255;
			}
		}
	}

	cvDilate(skin, skin);
}

void SkinDetector::detectSkinHist(IplImage * frame, IplImage * skin, CvRect & rect){
	cvZero(skin);
	skinHistogram.backproject(frame, rect);

	cvSetImageROI(skinHistogram.back, rect);
	cvSetImageROI(skin, rect);
	
	cvThreshold(skinHistogram.back, skin, 30, 255, CV_THRESH_BINARY);

	cvDilate(skin, skin);
	cvErode(skin, skin);

	cvResetImageROI(skin);
	cvResetImageROI(skinHistogram.back);
}

void SkinDetector::detectSkinAll(IplImage * frame, IplImage * skin, CvRect & rect){
	
	cvZero(skin);
	RgbImage rgb(frame);
	BwImage skin_img(skin);

	RgbPixel pix;
	
	for(int i = rect.y; i < rect.y+rect.height; ++i){
		for(int j = rect.x; j < rect.x+rect.width; ++j){

				pix = rgb[i][j];
				
				if(	pix.r > 95 &&
					pix.g > 40 &&
					pix.b > 20 &&
					(Utils::maximum(pix.r, pix.g, pix.b) - Utils::minimum(pix.r, pix.g, pix.b) 
							> 15) &&
					(abs(pix.r - pix.g) > 15)&&
					(pix.r > pix.b) &&
					(pix.r > pix.g)
					){
					skin_img[i][j] = (int)(120.0f*pix.r/160.0f);
			}
		}
	}

	skinHistogram.backproject(frame, rect);

	cvSetImageROI(skinHistogram.back, rect);
	cvSetImageROI(skin, rect);
	
	cvAdd(skin, skinHistogram.back, skin);
	cvThreshold(skin, skin, 150, 255, CV_THRESH_BINARY);

	cvDilate(skin, skin);
	cvErode(skin, skin);

	cvResetImageROI(skin);
	cvResetImageROI(skinHistogram.back);
}


void SkinDetector::detectSkinBgR(IplImage * frame, IplImage * skin, CvRect & rect, IplImage * fg){
	cvZero(skin);

	cvSetImageROI(frame, rect);
	cvSetImageROI(fg, rect);
	cvSetImageROI(skin, rect);

	cvSetImageROI(channel0, rect);
	cvSetImageROI(channel1, rect);
	cvSetImageROI(channel2, rect);

	// r - channel2
	// g - channel1
	// b - channel0
	cvSplit(frame, channel0, channel1, channel2, NULL);

	cvThreshold(channel2, skin, 40, 255, CV_THRESH_BINARY);
	cvAnd(skin, fg, skin);

	cvResetImageROI(channel0);
	cvResetImageROI(channel1);
	cvResetImageROI(channel2);

	cvResetImageROI(frame);
	cvResetImageROI(fg);
	cvResetImageROI(skin);
}

void SkinDetector::detectSkinBg(IplImage * frame, IplImage * skin, CvRect & rect, IplImage * fg){
	cvZero(skin);

	cvSetImageROI(frame, rect);
	cvSetImageROI(fg, rect);
	cvSetImageROI(skin, rect);

	cvCvtColor(frame, skin, CV_BGR2GRAY);

	cvThreshold(skin, skin, 100, 255, CV_THRESH_BINARY);

	cvAnd(skin, fg, skin);

	cvResetImageROI(frame);
	cvResetImageROI(fg);
	cvResetImageROI(skin);
}


/*
void SkinDetector::detectSkinFromHistogram(IplImage * frame, CvRect & rect){
	cvZero(skinMask);
	skinHistogram.backproject(frame, rect);

	cvSetImageROI(skinHistogram.back, rect);
	cvSetImageROI(skinMask, rect);
	
	cvThreshold(skinHistogram.back, skinMask, 30, 255, CV_THRESH_BINARY);

	cvDilate(skinMask, skinMask);
	cvErode(skinMask, skinMask);

	cvResetImageROI(skinMask);
	cvResetImageROI(skinHistogram.back);
}


void SkinDetector::detectSkinRGB(IplImage * frame, CvRect & rect){
	cvZero(skinMask);

	cvSetImageROI(skinMask, rect);
	cvSetImageROI(frame, rect);
	cvSetImageROI(maskTemp, rect);

	cvSetImageROI(channel0, rect);
	cvSetImageROI(channel1, rect);
	cvSetImageROI(channel2, rect);

	// r - channel2
	// g - channel1
	// b - channel0
	cvSplit(frame, channel0, channel1, channel2, NULL);

	// r > 95, g > 40, b > 20
	cvInRangeS(frame, cvScalar(20, 40, 95), cvScalar(250, 250, 250), skinMask);

	// r > g
	cvCmp(channel2, channel1, maskTemp, CV_CMP_GT);
	cvAnd(skinMask, maskTemp, skinMask);

	// r > b
	cvCmp(channel2, channel0, maskTemp, CV_CMP_GT);
	cvAnd(skinMask, maskTemp, skinMask);

	// r-g > 15
	cvSub(channel2, channel1, maskTemp);
	cvThreshold(maskTemp, maskTemp, 15, 255, CV_THRESH_BINARY);
	cvAnd(skinMask, maskTemp, skinMask);


	cvResetImageROI(maskTemp);
	cvResetImageROI(skinMask);
	cvResetImageROI(frame);

	cvResetImageROI(channel0);
	cvResetImageROI(channel1);
	cvResetImageROI(channel2);
}

void SkinDetector::detectSkinHSV(IplImage * frame, CvRect & rect){
	cvZero(skinMask);

	cvSetImageROI(skinMask, rect);
	cvSetImageROI(frame, rect);
	cvSetImageROI(temp, rect);
	
	cvCvtColor(frame, temp, CV_BGR2HSV);

	cvInRangeS(temp, cvScalar(0, 30, 80), cvScalar(20, 150, 255), skinMask);

	cvResetImageROI(temp);
	cvResetImageROI(skinMask);
	cvResetImageROI(frame);
}


void SkinDetector::detectSkin(IplImage * frame, CvRect & rect, int type){

}
*/