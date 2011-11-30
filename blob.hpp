#pragma once

#include <cv.h>
#include <deque>

class Blob
{
public:
	Blob(void);
	~Blob(void);

	CvRect getBiggerRect();
	CvRect getSmallerRect();

	void setLastRect(CvRect & rect);
	void setLastPointWithZ(int z);

	std::vector<CvRect> allRect;
	std::deque<CvPoint3D32f> allXYZ;

	CvRect lastRect;
	CvPoint lastPoint;

	CvPoint3D32f lastXYZ;
	CvScalar color;
	int lastZ;

	CvPoint2D32f circlePoint;
	int radius;
	//CvPoint seedPoint;

	IplImage * mask;

	int state;
	static int plusRectSize;
	int minArea;
	int maxArea;

	const int id;
	static int idCounter;
};
