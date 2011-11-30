#pragma once

#include <cv.h>


#define RGB_ 1
#define HSV_ 2
#define HIST_ 3
#define ALL_ 4
#define BGSKIN_ 5

#define BM_ 1
#define SGBM_ 2
#define BMCUDA_ 3
#define MINE_ 4
#define MINE_BM_ 5
#define MINE_RND_ 6

#define RESULT_FAIL 0
#define RESULT_OK 1

#define STATE_CALIBRATION_BEFORE 0
#define STATE_START_CALIBRATION 1
#define STATE_LOAD_CALIBRATION 2
#define STATE_CALIBRATION 3
#define STATE_AFTER_CAILBRATION 4

#define STATE_BEFORE_HIST 0
#define STATE_START_HIST 1
#define STATE_GET_HIST 2
#define STATE_AFTER_HIST 3

#define STATE_BEFORE_BACK 0
#define STATE_BACK 1
#define STATE_AFTER_BACK 2

class Settings
{
public:
	static Settings & getInstance();
	~Settings(){}

	int detectionMethod;
	int stereoMethod;

	int stateStereo;
	int stateHist;
	int stateBack;

	static CvSize imageSize;

	void initGameTime();
	void updateFPS();

	void reinit();

	float fps;
	long framesCounter;

private:
	Settings();

	float getGameTime();
	
	float lastUpdate;
	float fpsUpdateInterval;
	int  numFrames;
	float timeAtGameStart;
	
	UINT64 ticksPerSecond;
};
