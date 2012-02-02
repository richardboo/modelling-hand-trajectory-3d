#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "point3d.h"
#include "modeltrajectory.h"
#include "singletrajectory.h"


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
	
	std::string fileName;

	void loadSample(std::ifstream & file);
	void prepareAllTrajectories();
	void recognize();

	void showStats();
	void saveStats();
	void setRealIndex(int index);

	static char * trajName[5];

private:
	void findMinMax(SingleTrajectory & traj);
	void recognizeSingleTraj(SingleTrajectory & traj);
	void showStatsForTrajectory(SingleTrajectory & traj);
	void smoothTrajectoryKalman(std::vector<Point3D> & original, std::vector<Point3D> & smoothed);

	void filterSample();
	void smoothTrajectory();
	void smoothTrajectoryTwice();
	
};
