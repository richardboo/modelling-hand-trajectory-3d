#pragma once

#include <vector>
#include "modeltrajectory.h"
#include "point3d.h"
#include "trajectorysample.h"

/**
 * Fabryka modelowych trajekorii. 
 * Tworzy trajektorie dopasowane do pobranej na podstawie punktow charakterystych.
 * Generuje pokazowe modelowe trajektorie.
 */
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
	void createLine(std::vector<Point3D> & vect, Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count = 200);
	void createCircle(std::vector<Point3D> & vect, Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count = 200);
	void createSpiral(std::vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count = 200);
	void createZigZag(std::vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count = 200);
	void createArc(std::vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count = 200);
	
	ModelTrajectory * generateTrajectory(int index, Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count = 200);
};
