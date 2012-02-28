#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "point3d.h"
#include "modeltrajectory.h"
#include "singletrajectory.h"

/**
 * Probka trajektorii. Zawiera pobrana trajektorie i wygladzone trajektorie.
 */
class TrajectorySample
{
public:
	TrajectorySample();
	~TrajectorySample();

	// trajektoria oryginalna
	std::vector<Point3D> originalPoints;

	SingleTrajectory filteredTrajectory;
	SingleTrajectory kalmanTrajectory;
	SingleTrajectory kalman3Trajectory;

	bool isReady;
	int realIndex;
	float minZ, maxZ;
	
	std::string fileName;
	std::stringstream statsStr;
	//std::string statsStr;

	void loadSample(std::ifstream & file);
	void prepareAllTrajectories();
	void recognize();

	void showStats();
	void saveStats();
	void setRealIndex(int index);

	static char * trajName[5];

private:
	void findMinMax(SingleTrajectory & traj);
	void findMinMaxZ();
	void recognizeSingleTraj(SingleTrajectory & traj);
	void showStatsForTrajectory(SingleTrajectory & traj);
	void smoothTrajectoryKalman(std::vector<Point3D> & original, std::vector<Point3D> & smoothed);

	void filterSample();
	void smoothTrajectory();
	void smoothTrajectoryTwice();
	
};
