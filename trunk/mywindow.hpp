#pragma once

#include <cv.h>
#include <highgui.h>

/**
 * Klasa obudowujaca okno wysiwetlajace obraz z biblioteki OopenCV.
 */
class MyWindow{

public:
	MyWindow(const char * theName);
	MyWindow(const char * theName, int width);
	~MyWindow();

	void showImage(IplImage * frame);
	void showHalfSizeImage(IplImage * frame);
	void hide();
	void setXY(int xx, int yy);

	const char * name;

private:
	
	int width;
	bool shown;
	int x, y;

	IplImage * smallerC, *smallerG;
};