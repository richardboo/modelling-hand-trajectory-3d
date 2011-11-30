#pragma once

#include <cv.h>
#include <highgui.h>

class MyWindow{

public:
	MyWindow(const char * theName);
	MyWindow(const char * theName, int width);
	~MyWindow();

	void showImage(IplImage * frame);
	void showHalfSizeImage(IplImage * frame);
	void hide();

	const char * name;

private:
	
	int width;

	IplImage * smallerC, *smallerG;
};