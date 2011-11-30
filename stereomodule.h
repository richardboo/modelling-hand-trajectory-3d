#pragma once

#include <opencv/cv.h>
#include <highgui.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/gpu/gpu.hpp>
#include <vector>

#include "blob.hpp"

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

	cv::StereoSGBM sgbm;
	CvStereoBMState * BMState;
	IplImage * disparityNotNormalized, * andImage;
	IplImage * onlyHandNormalized[2];

	Mat disp, disparityNNmat, 
		m_remap1, m_remap2, m_remapped1, m_remapped2;
/*
GPU
	gpu::StereoBM_GPU bm;
	gpu::StereoConstantSpaceBP csbp;

	Mat disp, disparityNNmat, 
		m_remap1, m_remap2, m_remapped1, m_remapped2;
	gpu::GpuMat d_disp; 
	gpu::GpuMat d_left, d_right;
	
	gpu::GpuMat d_remap1, d_remap2, d_remapped1, d_remapped2;
*/

};
