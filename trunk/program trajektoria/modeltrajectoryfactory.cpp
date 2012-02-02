#include "modeltrajectoryfactory.h"


ModelTrajectoryFactory::ModelTrajectoryFactory(void){
	trNum = 4;
}

ModelTrajectoryFactory::~ModelTrajectoryFactory(void){
	for(unsigned int i = 0; i < originalTrajectories.size(); ++i){
		delete originalTrajectories[i];
	}
}

void ModelTrajectoryFactory::generateOriginalTrajectories(int width, int height, int maxZ){

	for(unsigned int i = 0; i < originalTrajectories.size(); ++i){
		delete originalTrajectories[i];
	}
	originalTrajectories.clear();
	
	for(unsigned int i = 0; i < 5; ++i){
		ModelTrajectory * thewNewOne = new ModelTrajectory;
		originalTrajectories.push_back(thewNewOne);
	}

}

void ModelTrajectoryFactory::fitModelTrajectories(TrajectorySample * sample){

}


ModelTrajectory * ModelTrajectoryFactory::generateTrajectory(int index, int width, int height, 
	Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX){
	
	ModelTrajectory * model = new ModelTrajectory;
	model->index = index;

	switch(index){
	
		
	
	}
																 
	return NULL;
}