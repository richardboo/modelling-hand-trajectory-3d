#include "trajectorysample.h"
#include <opencv2/video/tracking.hpp>
#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;
using namespace cv;

char * TrajectorySample::trajName[5] = {"oryginalna ", "bez wygladzania ", "wyg. filtrem Kalmana ", "wyg. filtrem Kalmana x3 ", "rozpoznana modelowa "};

TrajectorySample::TrajectorySample(void)
{
	isReady = false;
}

TrajectorySample::~TrajectorySample(void)
{
}

void TrajectorySample::loadSample(ifstream & file){

	// ustawiana orginalna trajektoria
	int w, h, count;
	float x, y, z;
	char c;
	file >> w;
	file >> h;
	file >> count;

	for(int i = 0; i < count && !file.eof(); ++i){
		file >> x >> c >> y >> c >> z;
		if(x != -1.0f && y != -1.0f && z!= -1.0f){
			originalPoints.push_back(Point3D(x, y, z));
		}
	}
}

// obliczne:
// - trajektoria probkowana
// - trajektoria filtrowana kalmanem
// - trajektoria filtrowana kalmanem 3x
// obliczane minima i maksima trajektorii
void TrajectorySample::prepareAllTrajectories(){
	if(originalPoints.size() < 2)
		return;
	findMinMaxZ();
	filterSample();
	smoothTrajectory();
	smoothTrajectoryTwice();
	findMinMax(filteredTrajectory);
	findMinMax(kalmanTrajectory);
	findMinMax(kalman3Trajectory);
}

void TrajectorySample::recognize(){
	recognizeSingleTraj(filteredTrajectory);
	recognizeSingleTraj(kalmanTrajectory);
	recognizeSingleTraj(kalman3Trajectory);
}

void TrajectorySample::findMinMaxZ(){
	minZ = originalPoints[0].z;
	maxZ = originalPoints[0].z;
	for(unsigned int i = 0; i < originalPoints.size(); ++i){
		if(originalPoints[i].z < minZ)
			minZ = originalPoints[i].z;
		else if(originalPoints[i].z > maxZ)
			maxZ = originalPoints[i].z;
	}
}

void TrajectorySample::filterSample(){

	// wykonanie 200 probek

	// najpierw populacja probek przez interpolacje
	// tak, aby odleglosc z pomiedzy nimi byla co najwyzej diffZ

	float diffZ = (maxZ-minZ)/200.0f;

	vector<Point3D> vectTemp;
	for(unsigned int i = 0; i < originalPoints.size()-1; ++i){
		Point3D point1 = originalPoints[i];
		Point3D point2 = originalPoints[i+1];

		if(abs(point1.z - point2.z) <= diffZ){
			vectTemp.push_back(point1);
			continue;
		}

		vectTemp.push_back(point1);
		float div = abs(point2.z - point1.z)/diffZ;
		float difX = (point2.x - point1.x)/div;
		float difY = (point2.y - point1.y)/div;
		float difZ = (point2.z - point1.z)/div;

		for(int k = 1; k <= (int)div; ++k){
			vectTemp.push_back(Point3D(point1.x+k*difX, point1.y+k*difY, point1.z+k*difZ));
		}
	}
	// i dodanie ostatniego
	vectTemp.push_back(originalPoints[originalPoints.size()-1]);
	
	// mamy punkty w odleglosci co najwyzej zdiff
	// ale moze byc ich wiecej niz 200
	if(vectTemp.size() < 200){
		cout << "NIEEEEE" << endl;
		return;
	}

	cout << "przed usuwaniem co drugi: " << vectTemp.size() << endl;

	list<Point3D> temp2;
	// usuwamy te, ktore sa za czesto
	for(unsigned int i = 0; i < vectTemp.size()-2; i+=2){
	
		temp2.push_back(vectTemp[i]);
		if(abs(vectTemp[i].z - vectTemp[i+2].z) > diffZ){
			// usuwamy ten posrodku, jesli jest <= zdiff
			temp2.push_back(vectTemp[i+1]);
		}
	}
	temp2.push_back(vectTemp[vectTemp.size()-1]);

	cout << "po usuwaniu co drugi: " << temp2.size() << endl;

	int tooMuch = temp2.size()-200;
	list<Point3D>::iterator iter;
	while(tooMuch >= 200){
		
		int counter = 0;
		for (iter = temp2.begin(); iter != temp2.end(); ++iter) {
			if(counter%2 == 1 && counter != 0 && counter != temp2.size()-1){
				iter = temp2.erase(iter);
				--iter;
			}
			counter++;
		}

		tooMuch = temp2.size()-200;
	}

	if(tooMuch > 0){
		
		// mamy tooMuch nadal, ale juz mniej niz 100
		// no to teraz usuwamy po prostu co ktorys
		// co ktory?
		int thisOne = 0;
		float mod = 200.0f/tooMuch;
		if(mod >= 1.5f)
			thisOne = (int)mod+1;
		else
			thisOne = (int)mod;

		int counter = 0;
		for (iter = temp2.begin(); iter != temp2.end() && temp2.size() > 200; ++iter) {
			if(counter != 0 && counter != temp2.size()-1 && counter == thisOne){
				iter = temp2.erase(iter);
				--iter;
			}
			counter = (counter+1)%(thisOne+1);
		}

		while(temp2.size() > 200){
			srand(time(NULL));
			int nr = (rand() % (temp2.size()-1)) + 1; // nie pierwszy i nie ostatni
			iter = temp2.begin();
			advance(iter, nr);
			temp2.erase(iter);
		}
	}

	for (iter = temp2.begin(); iter != temp2.end(); ++iter) {
		filteredTrajectory.points.push_back(*iter);
	}



	/*for(unsigned int i = 0; i < originalPoints.size(); ++i){
		if(filteredTrajectory.points.size() >= 1){
			Point3D lastOne = filteredTrajectory.points.at(filteredTrajectory.points.size()-1);

			for(int k = 1; k < 5; ++k){
				filteredTrajectory.points.push_back(Point3D((k*originalPoints[i].x+(5-k)*lastOne.x)/5, 
												(k*originalPoints[i].y+(5-k)*lastOne.y)/5, 
												(k*originalPoints[i].z+(5-k)*lastOne.z)/5));
			}
		}
		filteredTrajectory.points.push_back(originalPoints.at(i));
	}*/

}

void TrajectorySample::smoothTrajectory(){
	smoothTrajectoryKalman(filteredTrajectory.points, kalmanTrajectory.points);
}

void TrajectorySample::smoothTrajectoryTwice(){
	vector<Point3D> temp;
	smoothTrajectoryKalman(kalmanTrajectory.points, temp);
	smoothTrajectoryKalman(temp, kalman3Trajectory.points);
}



void TrajectorySample::saveStats(){

}

void TrajectorySample::showStats(){
	
	cout << "--Statystyki rozpoznawania--" << endl << endl;
	cout << "* ilosc punktow oryginalnych:\t" << originalPoints.size() << endl;
	cout << "* ilosc punktow przefiltrowanych:\t" << filteredTrajectory.points.size() << endl;
	cout << endl;
	cout << "PRZWDZIWA trajektoria:\t" << realIndex << endl;
	cout << "ROZPOZNANA trajektoria:\t" << filteredTrajectory.recognizedIndex << ", " << kalmanTrajectory.recognizedIndex << ", " << kalman3Trajectory.recognizedIndex << endl;
	cout << endl;
	cout << "najmniejsza odleglosc dla rozpoznanej trajektorii:\t";

	float minDiff = -1.0f;
	int index = -1;
	if(filteredTrajectory.recognizedIndex == realIndex){
		if(minDiff < 0){
			minDiff = filteredTrajectory.diffRecognized;
			index = 0;
		}
	}
	if(kalmanTrajectory.recognizedIndex == realIndex){
		if(minDiff < 0 || kalmanTrajectory.diffRecognized < minDiff){
			minDiff = kalmanTrajectory.diffRecognized;
			index = 1;
		}
	}
	if(kalman3Trajectory.recognizedIndex == realIndex){
		if(minDiff < 0 || kalman3Trajectory.diffRecognized < minDiff){
			minDiff = kalman3Trajectory.diffRecognized;
			index = 2;
		}
	}
	if(minDiff > 0){
		cout << (index == 0 ? "nie wygladzona" : (index == 1 ? "kalman" : "3x kalman")) << endl;
	}
	else{
		cout << "brak rozpoznanej trajektorii" << endl;
	}

	cout << endl << "\t** NIE WYGLADZONA **" << endl;
	showStatsForTrajectory(filteredTrajectory);
	cout << endl << "\t** WYGLADZONA FILTREM KALMANA **"<<endl;
	showStatsForTrajectory(kalmanTrajectory);
	cout << endl << "\t** WYGLADZONA 3X FILTREM KALMANA **"<<endl;
	showStatsForTrajectory(kalman3Trajectory);
}

void TrajectorySample::showStatsForTrajectory(SingleTrajectory & traj){
	cout << "rozpoznana trajektoria:\t" << traj.recognizedIndex << endl;
	cout << "\tmodel\tdiff"<<endl;
	
	for(unsigned int i = 0; i < traj.diffFittedModels.size(); ++i){
		cout << "\t"<<ModelTrajectory::name[i]<<"\t"<<traj.diffFittedModels[i]<<endl;
	}
}

void TrajectorySample::setRealIndex(int index){
	realIndex = index;
}



void TrajectorySample::findMinMax(SingleTrajectory & traj){
	if(traj.points.size() < 1)
		return;

	traj.minX = traj.points[0];
	traj.minY = traj.points[0];
	traj.minZ = traj.points[0];
	traj.maxX = traj.points[0];
	traj.maxY = traj.points[0];
	traj.maxZ = traj.points[0];

	for(unsigned int i = 0; i < traj.points.size(); ++i){

		Point3D & curr = traj.points[i];
		if(curr.x < traj.minX.x){
			traj.minX = curr;
		}
		if(curr.y < traj.minY.y){
			traj.minY = curr;
		}
		if(curr.z < traj.minZ.z){
			traj.minZ = curr;
		}
		if(curr.x > traj.maxX.x){
			traj.maxX = curr;
		}
		if(curr.y > traj.maxY.y){
			traj.maxY = curr;
		}
		if(curr.z > traj.maxZ.z){
			traj.maxZ = curr;
		}
	}
}

void TrajectorySample::recognizeSingleTraj(SingleTrajectory & traj){

	float min = -1.0f;
	int minIndex = -1;

	// dla kazdej trajektorii modelowej liczenie odleglosci pomiedzy punktami
	for(unsigned int m = 0; m < traj.fittedModels.size(); ++m){
		
		ModelTrajectory * curr = traj.fittedModels[m];
		traj.diffFittedModels.push_back(0.0f);

		// punktow powinno byc tyle samo
		for(unsigned int i = 0; i < traj.points.size(); ++i){

			Point3D & org = traj.points[i];
			Point3D & fitted = curr->points[i];
			float diff = abs(org.x-fitted.x) + abs(org.y-fitted.y) + abs(org.z-fitted.z);
			traj.diffFittedModels[m] += diff;
		}

		// wybor minimalnej
		if(minIndex == -1 || traj.diffFittedModels[m] < min){
			minIndex = m;
			min = traj.diffFittedModels[m];
		}
	}

	// zapis rozpoznanej trajektorii
	traj.recognizedIndex = minIndex;
	traj.diffRecognized = min;
}

void TrajectorySample::smoothTrajectoryKalman(std::vector<Point3D> & original, std::vector<Point3D> & smoothed){
	
	cv::KalmanFilter KF(4, 2, 0);
    Mat_<float> state(4, 1); /* (x, y, Vx, Vy) */
    Mat processNoise(4, 1, CV_32F);
    Mat_<float> measurement(2,1); 
	
    char code = (char)-1;
	
	Point3D point = original.at(0);

	measurement.setTo(Scalar(point.x, point.y));

    KF.statePre.at<float>(0) = point.x;
	KF.statePre.at<float>(1) = point.y;
	KF.statePre.at<float>(2) = 0;
	KF.statePre.at<float>(3) = 0;
	KF.transitionMatrix = *(Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
	
    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(KF.errorCovPost, Scalar::all(5));

	for(unsigned int i = 1; i < original.size() ;++i)
    {	
		Point3D point = original.at(i);

        Mat prediction = KF.predict();
		Point2f predictPt(prediction.at<float>(0),prediction.at<float>(1));
		
        measurement(0) = point.x;
		measurement(1) = point.y;
		
		Point2f measPt(measurement(0),measurement(1));

		Mat estimated = KF.correct(measurement);
		smoothed.push_back(Point3D(estimated.at<float>(0),estimated.at<float>(1), point.z));
	}

}