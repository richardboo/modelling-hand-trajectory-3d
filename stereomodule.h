#pragma once

#include <opencv/cv.h>
#include <highgui.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <opencv2/gpu/gpu.hpp>
#include <vector>

#include "blob.hpp"
#include "myhandbm.h"
#include "opencv\stereovar.h"

using namespace std;
using namespace cv; 

class StereoModule
{
public:
	StereoModule(void);
	~StereoModule(void);

	void stereoStart(CvSize & size);
	int stereoProcessGray(IplImage* rectifiedGray[2], IplImage * blobs[2], Blob * hands[2], IplImage * disparity, int type);
	void stereoProcessMine(IplImage* rectified[2], IplImage * blobs[2], Blob * hands[2], IplImage * disparity, int type);

	static void initStates();

	static cv::StereoSGBM sgbm;
	static StereoVar vars;
	static CvStereoBMState BMState;
	static CvStereoBMState BMStateCuda;
	static MyHandBM myHandBMState;
	IplImage * disparityNotNormalized, * andImage;
	IplImage * onlyHandNormalized[2];
	IplImage * handGray[2];

	FastFeatureDetector * detector;
	BriefDescriptorExtractor * extractor;
	BruteForceMatcher<Hamming> * matcher_popcount;
	std::vector<KeyPoint> kpts_1, kpts_2;
	std::vector<DMatch> matches_popcount;
	std::vector<Point2f> mpts_1, mpts_2;

	// 8 - max disparity dla moich alg
	//IplImage * diffImage[8];

	Mat disp, disparityNNmat, 
		m_remap1, m_remap2, m_remapped1, m_remapped2;

//GPU
	/*
	gpu::StereoBM_GPU bm;
	gpu::StereoConstantSpaceBP csbp;

	gpu::GpuMat d_disp; 
	gpu::GpuMat d_left, d_right;
	
	gpu::GpuMat d_remap1, d_remap2, d_remapped1, d_remapped2;
	*/

};
