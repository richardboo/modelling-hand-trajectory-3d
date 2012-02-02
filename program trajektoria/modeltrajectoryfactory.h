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

	

	std::vector<ModelTrajectory *> originalTrajectories;
	void fitModelTrajectories(TrajectorySample * sample);

	int trNum;

private:
	void createLine(std::vector<Point3D> & vect, Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX);
	void createCircle(std::vector<Point3D> & vect, Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX);
	void createSpiral(std::vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX);
	void createZigZag(std::vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX);
	void createArc(std::vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX);
	
	ModelTrajectory * generateTrajectory(int index, Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX);
};
