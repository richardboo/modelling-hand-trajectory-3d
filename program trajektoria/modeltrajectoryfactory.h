#pragma once

#include <vector>
#include "modeltrajectory.h"
#include "point3d.h"
#include "trajectorysample.h"


class ModelTrajectoryFactory
{
public:
	ModelTrajectoryFactory(void);
	~ModelTrajectoryFactory(void);

	void generateOriginalTrajectories(int width, int height, int maxZ);

	ModelTrajectory * generateTrajectory(int index, int width, int height, 
		Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX);

	std::vector<ModelTrajectory *> originalTrajectories;
	void fitModelTrajectories(TrajectorySample * sample);

	int trNum;
	
};
