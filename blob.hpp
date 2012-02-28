#pragma once

#include <cv.h>
#include <deque>

/**
 * Klasa reprezentujaca obiekt polaczony wykryty na obrazie.
 * Zawiera wektor ostatnich polozen.
 */
class Blob
{
public:
	Blob(void);
	~Blob(void);

	CvRect getBiggerRect(int plus);
	CvRect getSmallerRect();

	void setLastRect(CvRect & rect);
	void setLastPointWithZ(int z);
	void setRealPoint(float x, float y, float z);

	std::vector<CvRect> allRect;
	std::deque<CvPoint3D32f> allXYZ;
	std::deque<CvPoint3D32f> allXYZReal;

	CvRect lastRect;
	int lastDisp;
	CvPoint lastPoint;

	CvRect lastKnownRect;

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

	bool occluded;

	const int id;
	static int idCounter;
};
