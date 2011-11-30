#include "histogram.hpp"
#include "settings.h"

Histogram::Histogram() {

	histUpdateCounter = 0;
 
    hbins = 30;
    sbins = 32;
    int histSize[] = {hbins, sbins};

	float hranges[] = { 0, 180 };
    float sranges[] = { 0, 256 };
    float * ranges[] = { hranges, sranges };
     
	hist = cvCreateHist(	2, //number of hist dimensions
                            histSize, //array of dimension sizes
							CV_HIST_ARRAY, //representation format
                            ranges, //array of ranges for bins
                            1); //uniformity flag

	CvSize size = Settings::instance()->defSize;
	hsv =		cvCreateImage(size, 8, 3);
	splane =	cvCreateImage(size, 8, 1);
	hplane =	cvCreateImage(size, 8, 1);
	vplane =	cvCreateImage(size, 8, 1);
	mask =		cvCreateImage(size, 8, 1);
	
	back = cvCreateImage(size, 8, 1);
}


Histogram::~Histogram() {
	cvReleaseImage(&hsv);
	cvReleaseImage(&splane);
	cvReleaseImage(&hplane);
	cvReleaseImage(&vplane);
	cvReleaseImage(&back);
}


// tworzenie histogramu - update, zbieranie informacji z rect
// frame jest RGB
void Histogram::update(IplImage * frame, CvRect & rect){

	// zerujemy maske twarzy
	// i nakladamy nowa maske
	cvZero(mask);
	cvSetImageROI(mask, rect);
	cvSet(mask, cvScalarAll(255));
	cvResetImageROI(mask);

	cvCvtColor(frame, hsv, CV_BGR2HSV);
	cvCvtPixToPlane(hsv, hplane, splane, vplane, NULL);	

	IplImage* planes[] = { hplane, splane };

	float maxVal;
	cvCalcHist(planes, hist, 1, mask);
	cvGetMinMaxHistValue( hist, 0, &maxVal, 0, 0 );
    cvConvertScale( hist->bins, hist->bins, maxVal ? 255. / maxVal : 0., 0 );

	histUpdateCounter++;
}

// po utworzeniu histogramu
// tworzy backproject na podanym rect
void Histogram::backproject(IplImage * frame, CvRect & rect){
	//IplImage * back = cvCreateImage(cvSize(rect.width, rect.height), 8, 1);

	cvCvtColor(frame, hsv, CV_BGR2HSV);
	cvZero(back);
	//cvSmooth(hsv, hsv);

	cvCvtPixToPlane(hsv, hplane, splane, vplane, NULL);	
	IplImage* planes[] = { hplane, splane };

	cvSetImageROI(hplane, rect);
	cvSetImageROI(splane, rect);
	cvSetImageROI(back, rect);

	cvCalcBackProject(planes, back, hist);

	cvSmooth(back, back);

	cvResetImageROI(hplane);
	cvResetImageROI(splane);
	cvResetImageROI(back);
}

/*
void Histogram::update(const Mat& image) {

    float hranges[] = { 0, 180 };
    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges, sranges };
    int histSize[] = {hbins, sbins};
    int channels[] = {0, 1};
    
    if (first) {
        calcHist( &image,  1, channels, Mat(), histogram, 2,  histSize, ranges,  true, false );
        normalize(histogram, histogram, 255);
        new_hist = histogram;
        first = false;
    } else {
        old_hist = new_hist;
        calcHist( &image,  1, channels, Mat(), new_hist, 2,  histSize, ranges,  true, false );
        assert(new_hist.type() == old_hist.type());
        //double diff = compareHist(new_hist, old_hist, CV_COMP_BHATTACHARYYA);
        // TODO: check if this is actually correct:
        add(new_hist, old_hist, histogram);
        //histogram = new_hist;
        normalize(histogram, histogram, 255);
    }
}


Mat Histogram::backproject(const Mat& image) {
    float hranges[] = { 0, 180 };
    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges, sranges };
    int channels[] = {0, 1};
    
    Mat backprojection;

    calcBackProject( &image, 1, channels, histogram, backprojection, ranges );
    return backprojection;
}
*/