#pragma once

#include "cv.h"

using namespace cv;

class Histogram {

public:
    Histogram();
    ~Histogram();

	// tworzenie histogramu - update, zbieranie informacji
    void update(IplImage * frame, CvRect & rect);

	// po utworzeniu histogramu
	// tworzy backproject
    void backproject(IplImage * frame, CvRect & rect);

	int getHistUpdateCounter() { return histUpdateCounter; }
	IplImage * back;
    
private:

	IplImage* hsv; //input image converted to HSV
	IplImage* hplane; //hue channel of HSV image
	IplImage* splane; //hue channel of HSV image
	IplImage* vplane;

	IplImage* mask; //image for masking pixels

	CvHistogram* hist; //histogram of hue in original face image

    int hbins;
    int sbins;
    float* hranges;
    float* sranges;
    float** ranges;
    int* histSize;
    int* channels;

	int histUpdateCounter;
};

