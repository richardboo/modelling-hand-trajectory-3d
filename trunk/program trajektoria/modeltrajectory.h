#pragma once

#include <vector>
#include <string>

#include "point3d.h"

class ModelTrajectory
{
public:
	ModelTrajectory();
	ModelTrajectory(int index_): index(index){} 
	~ModelTrajectory();

	int index;
	std::vector<Point3D> points;

	// stale
	static const int LINE;
	static const int SPIRAL;
	static const int CIRCLE;
	static const int ZIGZAG;
	static const int ARC;

	static char * name[5];
};
