#pragma once

#include <opencv/cv.h>
#include <highgui.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

/**
 * Algorytm StereoVar z biblioteki OpenCV.
 */
class StereoVar
    {
    public:
        // Flags	
        enum {USE_INITIAL_DISPARITY = 1, USE_EQUALIZE_HIST = 2, USE_SMART_ID = 4, USE_MEDIAN_FILTERING = 8};
        enum {CYCLE_O, CYCLE_V};
        enum {PENALIZATION_TICHONOV, PENALIZATION_CHARBONNIER, PENALIZATION_PERONA_MALIK};
        
        //! the default constructor
        StereoVar();
        
        //! the full constructor taking all the necessary algorithm parameters
        StereoVar(int levels, double pyrScale, int nIt, int minDisp, int maxDisp, int poly_n, double poly_sigma, float fi, float lambda, int penalization, int cycle, int flags);
        
        //! the destructor
        virtual ~StereoVar();
        
        //! the stereo correspondence operator that computes disparity map for the specified rectified stereo pair
        virtual void operator()(const Mat& left, const Mat& right, Mat& disp);
        
        int		levels;
        double	pyrScale;
        int		nIt;
        int		minDisp;
        int		maxDisp;
        int		poly_n;
        double	poly_sigma;
        float	fi;
        float	lambda;
        int		penalization;
        int		cycle;
        int		flags;
        
    private:
        void FMG(Mat &I1, Mat &I2, Mat &I2x, Mat &u, int level);
        void VCycle_MyFAS(Mat &I1_h, Mat &I2_h, Mat &I2x_h, Mat &u_h, int level);
        void VariationalSolver(Mat &I1_h, Mat &I2_h, Mat &I2x_h, Mat &u_h, int level);
    };
    
 void polyfit(const Mat& srcx, const Mat& srcy, Mat& dst, int order);
