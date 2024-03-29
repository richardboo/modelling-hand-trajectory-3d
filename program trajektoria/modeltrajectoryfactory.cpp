#include "modeltrajectoryfactory.h"
#include <math.h>

#define MIN_VALUE 80

using namespace std;

ModelTrajectoryFactory::ModelTrajectoryFactory(void){
	trNum = 4;
}

ModelTrajectoryFactory::~ModelTrajectoryFactory(void){
	for(unsigned int i = 0; i < originalTrajectories.size(); ++i){
		delete originalTrajectories[i];
	}
}

void ModelTrajectoryFactory::generateOriginalTrajectories(int width, int height, int _maxZ){

	for(unsigned int i = 0; i < originalTrajectories.size(); ++i){
		delete originalTrajectories[i];
	}
	originalTrajectories.clear();
	
	for(unsigned int i = 0; i < 5; ++i){
		ModelTrajectory * thewNewOne = new ModelTrajectory;
		thewNewOne->index = i;
		originalTrajectories.push_back(thewNewOne);
	}

	float defX = (float)width/2.0f;
	float defY = (float)height/2.0f;
	float defZ = 200.0f;

	Point3D minX((float)width/3.0f,		defY,					defZ);
	Point3D minY(defX,					(float)height/3.0f,		defZ);
	Point3D minZ(defX,					defY,					100.0f);
	Point3D maxX(2.0f*(float)width/3.0f,defY,					defZ);
	Point3D maxY(defX,					2.0f*(float)height/3.0f,defZ);
	Point3D maxZ(defX,					defY,					300.0f);

	// line
	createLine(originalTrajectories[0]->points, maxZ, minZ, maxY, minY, maxX, minX);
	createSpiral(originalTrajectories[1]->points, maxZ, minZ, maxY, minY, maxX, minX);
	createCircle(originalTrajectories[2]->points, maxZ, minZ, maxY, minY, maxX, minX);
	createZigZag(originalTrajectories[3]->points, maxZ, minZ, maxY, minY, maxX, minX);
	createArc(originalTrajectories[4]->points, maxZ, minZ, maxY, minY, maxX, minX);
}

void ModelTrajectoryFactory::fitModelTrajectories(TrajectorySample * sample){

	// filtrowane
	SingleTrajectory & single = sample->filteredTrajectory;
	single.fittedModels.clear();
	
	for(unsigned int i = 0; i < 5; ++i){
		ModelTrajectory * thewNewOne = generateTrajectory(i, single.maxZ, single.minZ, single.maxY, single.minY, single.maxX, single.minX);
		single.fittedModels.push_back(thewNewOne);
	}
	
	// kalman
	SingleTrajectory & single2 = sample->kalmanTrajectory;
	single2.fittedModels.clear();
	
	for(unsigned int i = 0; i < 5; ++i){
		ModelTrajectory * thewNewOne = generateTrajectory(i, single2.maxZ, single2.minZ, single2.maxY, single2.minY, single2.maxX, single2.minX, single2.points.size());
		single2.fittedModels.push_back(thewNewOne);
	}

	// kalman 3
	SingleTrajectory & single3 = sample->kalman3Trajectory;
	single3.fittedModels.clear();
	
	for(unsigned int i = 0; i < 5; ++i){
		ModelTrajectory * thewNewOne = generateTrajectory(i, single3.maxZ, single3.minZ, single3.maxY, single3.minY, single3.maxX, single3.minX, single3.points.size());
		single3.fittedModels.push_back(thewNewOne);
	}

}


ModelTrajectory * ModelTrajectoryFactory::generateTrajectory(int index, 
	Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count){
	
	ModelTrajectory * model = new ModelTrajectory;
	model->index = index;

	switch(index){
		case 0:
		createLine(model->points, maxZ, minZ, maxY, minY, maxX, minX, count);
		break;

		case 1:
		createSpiral(model->points, maxZ, minZ, maxY, minY, maxX, minX, count);
		break;

		case 2:
		createCircle(model->points, maxZ, minZ, maxY, minY, maxX, minX, count);
		break;

		case 3:
		createZigZag(model->points, maxZ, minZ, maxY, minY, maxX, minX, count);
		break;

		case 4:
		createArc(model->points, maxZ, minZ, maxY, minY, maxX, minX, count);
		break;
	}
																 
	return model;
}

void ModelTrajectoryFactory::createLine(vector<Point3D> & vect,	Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count){

	float x, y, zdiff, currZ;

	if(maxZ.z-minZ.z < MIN_VALUE){
		zdiff = MIN_VALUE/(float)count;
		currZ = (MIN_VALUE - (maxZ.z-minZ.z))/2.0f;
	}
	else{
		zdiff = (maxZ.z-minZ.z)/(float)count;
		currZ = minZ.z;
	}

	x = (maxX.x-minX.x)/2.0f + minX.x;
	y = (maxY.y-minY.y)/2.0f + minY.y;
	//x = (maxZ.x-minZ.x)/2.0f + minX.x;
	//y = (maxZ.y-minZ.y)/2.0f + minY.y;
	
	for(int i = 0; i < count; ++i){
		vect.push_back(Point3D(x,y,currZ));
		currZ+=zdiff;
	}
}

void ModelTrajectoryFactory::createCircle(vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count){
	
	float z = (maxZ.z-minZ.z)/2.0f+minZ.z;
	float x, y;
	x = (maxX.x-minX.x)/2.0f + minX.x;
	y = (maxY.y-minY.y)/2.0f + minY.y;
	
	float angle = 0;
	float angleDiff = 360.0f/(float)count;
	float rad = 0;
	float divider = 3.14159265f/180.f;
	float radius = ((maxX.x-minX.x)/2.0f + (maxY.y-minY.y)/2.0f)/2.0f;
	if(radius < MIN_VALUE/2.0f)
		radius = MIN_VALUE/2.0f;

	for(int i = 0; i < count; ++i){
		rad = angle*divider;
		vect.push_back(Point3D(sin(rad)*radius+x, cos(rad)*radius+y, z));
		angle += angleDiff;
	}
}

void ModelTrajectoryFactory::createSpiral(vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count){

	float x, y;
	x = (maxX.x-minX.x)/2.0f + minX.x;
	y = (maxY.y-minY.y)/2.0f + minY.y;
	
	float angle = 0;
	float angleDiff = 540.0f/(float)count;
	float rad = 0;
	float divider = 3.14159265f/180.f;
	float radius = ((maxX.x-minX.x)/2.0f + (maxY.y-minY.y)/2.0f)/2.0f;
	
	if(radius < MIN_VALUE/2.0f)
		radius = MIN_VALUE/2.0f;

	float diffZ, currZ;

	if(maxZ.z-minZ.z < MIN_VALUE){
		diffZ = MIN_VALUE/(float)count;
		currZ = (MIN_VALUE - (maxZ.z-minZ.z))/2.0f;
	}
	else{
		diffZ = (maxZ.z-minZ.z)/(float)count;
		currZ = minZ.z;
	}

	for(int i = 0; i < count; ++i){
		rad = angle*divider;
		vect.push_back(Point3D(sin(-rad)*radius+x, cos(rad)*radius+y, currZ));
		angle += angleDiff;
		currZ += diffZ;
	}

}

void ModelTrajectoryFactory::createZigZag(vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count){

	// z half - gdzie jest przelamanie
	// poznawane po maksymalnym wychyleniu x
	float zHalf = maxX.z;

	// sredni y z wychylenia
	float y = (maxY.y-minY.y)/2.0f + minY.y;

	float zdiff, currZ;
	if(maxZ.z-minZ.z < MIN_VALUE){
		zdiff = MIN_VALUE/(float)count;
		currZ = (MIN_VALUE - (maxZ.z-minZ.z))/2.0f;
	}
	else{
		zdiff = (maxZ.z-minZ.z)/(float)count;
		currZ = minZ.z;
	}


	float divide = ((zHalf-minZ.z)/(maxZ.z-minZ.z)) * count;
	// jesli nie jest to posrodku
	if(divide < 70.0f || divide > 130.0f){
		divide = 100.0f;
	}

	float currX = minX.x;
	float realxdiff = (maxX.x-minX.x);

	if(realxdiff < MIN_VALUE){
		realxdiff = MIN_VALUE;
	}

	float diffX = realxdiff/divide;
	
	
	// x rosnie od minX do maxX o diff
	for(int i = 0; i < (int)divide; ++i){
		vect.push_back(Point3D(currX, y, currZ));
		currX+=diffX;
		currZ+=zdiff;
	}

	//divide = (float)count-divide;
	//if(divide == 0)	divide = 1.0f;
	diffX = realxdiff/((float)count-divide);

	//currX = minX.x+diffX*(int)divide;

	// x maleje
	for(int i = (int)divide; i < count; ++i){
		vect.push_back(Point3D(currX, y, currZ));
		currX-=diffX;
		currZ+=zdiff;
	}

}

void ModelTrajectoryFactory::createArc(vector<Point3D> & vect,Point3D & maxZ, Point3D & minZ, Point3D & maxY, Point3D & minY, Point3D & maxX, Point3D & minX, int count){

	float z = (maxZ.z-minZ.z)/2.0f + minZ.z;
	float y = minY.y;

	float x = (maxX.x-minX.x)/2.0f + minX.x;
	float angle = 270.0f;
	float angleDiff = (450.0f-270.0f)/(float)count;
	float rad = 0;
	float divider = 3.14159265f/180.f;

	float radiusZ, radiusY;
	radiusZ = (maxZ.z-minZ.z)/2.0f;
	radiusY = (maxY.y-minY.y);

	if(radiusZ < MIN_VALUE/2.0f)
			radiusZ = MIN_VALUE/2.0f;
	if(radiusY < MIN_VALUE/2.0f)
			radiusY = MIN_VALUE/2.0f;
	//= ((maxZ.z-minZ.z)/2.0f + (maxY.y-minY.y)/2.0f)/2.0f;
/*
	float plusY, plusZ;
	if(maxZ.z-minZ.z > maxY.y-minY.y){
		plusY = 0.0f;
		radius = (maxY.y-minY.y)/2.0f;
		if(radius < MIN_VALUE/2.0f)
			radius = MIN_VALUE/2.0f;
		plusZ = ((maxZ.z-minZ.z)/2.0f - radius)/(float)count;
	}
	else{
		plusZ = 0.0f;
		radius = (maxZ.z-minZ.z)/2.0f;
		if(radius < MIN_VALUE/2.0f)
			radius = MIN_VALUE/2.0f;
		plusY = ((maxY.y-minY.y)/2.0f - radius)/(float)count;
	}
*/

	for(int i = 0; i < count; ++i){
		rad = angle*divider;
		vect.push_back(Point3D(x, cos(rad)*radiusY+y, sin(rad)*radiusZ+z));
		angle += angleDiff;
	}

}