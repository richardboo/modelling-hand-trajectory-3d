#pragma once

#include <vector>

#include "point3d.h"
#include "modeltrajectory.h"

/**
 * Pojedynczy obiekt trajektorii pobranej.
 * Zawiera trajektorie pobrana, prawdziwy index, trajektorie modelowe dopasowe i wyniki rozpoznawania.
 */
class SingleTrajectory
{
public:
	SingleTrajectory(void){}
	~SingleTrajectory(void){}

	std::vector<Point3D> points;
	Point3D maxZ, minZ, maxX, minX, maxY, minY;
	std::vector<ModelTrajectory *> fittedModels;

	int realIndex;
	int recognizedIndex;
	float diffRecognized;
	std::vector<float> diffFittedModels;
};
