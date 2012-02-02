#include "modeltrajectory.h"

const int ModelTrajectory::LINE		= 0;
const int ModelTrajectory::SPIRAL	= 1;
const int ModelTrajectory::CIRCLE	= 2;
const int ModelTrajectory::ZIGZAG	= 3;
const int ModelTrajectory::ARC		= 4;

char * ModelTrajectory::name[5] = {"line", "spiral", "circle", "zigzag", "arc"};

ModelTrajectory::ModelTrajectory(void)
{
}

ModelTrajectory::~ModelTrajectory(void)
{
}
