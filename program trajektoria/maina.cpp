
#include <iostream>
#include <fstream>
#include <vector>

#include <windows.h>
#include <GL/glut.h>

#include <opencv2/video/tracking.hpp>
#include <cv.h>

#include <math.h>
#include <stdio.h>
 
#define PI 3.14

#define LINE 0
#define SPIRAL 1
#define ZIGZAG 2
#define UP 3

using namespace std;
using namespace cv;

class Point3D{
public:
	Point3D(): x(-1), y(-1), z(-1){}
	Point3D(int xx, int yy, int zz): x(xx), y(yy), z(zz){}
	int x, y, z;
};

vector<Point3D> trajectory;
vector<Point3D> trajectorySmooth;
vector<Point3D> trajectoryModel;

vector<Point3D> one;

bool keyStates[256]; // Create an array of boolean values of length 256 (0-255)  
bool showNotKalman = false;
 //angle of rotation
float xpos = 0, ypos = 0, zpos = 0, xrot = 0, yrot = 0, angle=0.0;
float lastx, lasty;
float scale = 0.008f;

float w = 640.0f;
float h = 480.0f;

OPENFILENAME ofn;       // common dialog box structure
char szFile[260];       // buffer for file name
HWND hwnd = NULL;              // owner window
HANDLE hf;              // file handle

int lineXbegin = 100;
int lineYbegin = 300;
int maxZ = 300;
int circleRadius = 70;
int startangle=0,endangle=540;
int zbegin, zend;




void renderPrimitive (void) {
	//glRotatef(10, 0.0f, 1.0f, 0.0f);
	//glRotatef(10, -1.0f, 0.0f, 0.0f);
	//glRotatef(10, 0.0f, 1.0f, 0.0f);
	//glRotatef(10, 0.0f, 0.0f, -1.0f);

	glScalef(scale,scale,-scale);
	
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS); // Start drawing a quad primitive  
  
	glVertex3f(0.0f, 0.0f, 0.0f); // The bottom left corner  
	glVertex3f(0.0f, 0.0f, 300.0f); // The top left corner  
	glVertex3f(640.0f, 0.0f, 300.0f); // The top right corner  
	glVertex3f(640.0f, 0.0f, 0.0f); // The bottom right corner

	glVertex3f(0.0f, 480.0f, 0.0f); // The bottom left corner  
	glVertex3f(0.0f, 480.0f, 300.0f); // The top left corner  
	glVertex3f(640.0f, 480.0f, 300.0f); // The top right corner  
	glVertex3f(640.0f, 480.0f, 0.0f); // The bottom right corner


	glColor3f(0.7f, 0.7f, 0.7f);
	glVertex3f(0.0f, 0.0f, 300.0f); // The bottom left corner  
	glVertex3f(0.0f, 480.0f, 300.0f); // The top left corner  
	glVertex3f(640.0f, 480.0f, 300.0f); // The top right corner  
	glVertex3f(640.0f, 0.0f, 300.0f); // The bottom right corner  
	  
	glEnd(); 

	if(showNotKalman){

		glBegin(GL_LINES);
		glPointSize(2.0f);  
		glColor3f(1.0f, 0.0f, 0.0f);
		//glVertex3f(10, 10, 0);
		
		for(int i = 0; i < trajectory.size()-1; i++){
			glVertex3f(trajectory[i].x, trajectory[i].y, trajectory[i].z);
			glVertex3f(trajectory[i+1].x, trajectory[i+1].y, trajectory[i+1].z);
		}
		
		glEnd();
	}

	glBegin(GL_LINES);
	glPointSize(2.0f);  
	glColor3f(0.0f, 0.0f, 1.0f);
	
	for(int i = 0; i < trajectorySmooth.size()-1; i++){
		glVertex3f(trajectorySmooth[i].x, trajectorySmooth[i].y, trajectorySmooth[i].z);
		glVertex3f(trajectorySmooth[i+1].x, trajectorySmooth[i+1].y, trajectorySmooth[i+1].z);
	}
	glEnd();

	glBegin(GL_LINES);
	glPointSize(2.0f);  
	glColor3f(0.0f, 1.0f, 0.0f);

	if(!trajectoryModel.empty()){
		for(int i = 0; i < trajectoryModel.size()-1; i++){
			glVertex3f(trajectoryModel[i].x, trajectoryModel[i].y, trajectoryModel[i].z);
			glVertex3f(trajectoryModel[i+1].x, trajectoryModel[i+1].y, trajectoryModel[i+1].z);
		}
	}
	glEnd();
}  
  
 
void keyUp (unsigned char key, int x, int y);
void keyPressed (unsigned char key, int x, int y);
void reshape (int width, int height);
void display (void);
void keyOperations (void);
bool loadFile();
bool openFile();
void mouseMovement(int x, int y);
void mouseGo(int, int);
vector<Point3D> smoothTrajectory(vector<Point3D>& vect);
void loadModelTrajectory(int model);
void frontArc(int cx, int cy, int radius);
void circle(int cx, int cy, int radius);
void fitToModel(int nr);

  
int main (int argc, char **argv) {  
	if(openFile()){
		loadFile();
		one = smoothTrajectory(trajectory);
		trajectorySmooth = smoothTrajectory(one);
	}
	for(int i = 0; i < 256; ++i)
		keyStates[i] = false;

	glutInit(&argc, argv); // Initialize GLUT 
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH); 
	glutInitWindowSize (640, 640); // Set the width and height of the window  
	glutInitWindowPosition (100, 100); // Set the position of the window  
	glutCreateWindow ("trajectory"); // Set the title for the window  
	
	glutDisplayFunc(display); // Tell GLUT to use the method "display" for rendering  
	glutIdleFunc (display);   

	glutReshapeFunc(reshape); // Tell GLUT to use the method "reshape" for reshaping  
	
	glutMotionFunc(mouseMovement); //check for mouse
	glutPassiveMotionFunc(mouseGo);

	glutKeyboardFunc(keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
	glutKeyboardUpFunc(keyUp); // Tell GLUT to use the method "keyUp" for key up events  
	  
	glutMainLoop(); // Enter GLUT's main loop  
}  

void keyPressed (unsigned char key, int x, int y) {  
	keyStates[key] = true; // Set the state of the current key to pressed  
}  

void keyUp (unsigned char key, int x, int y) {  
	keyStates[key] = false; // Set the state of the current key to not pressed  
} 

void display (void) {  
	keyOperations();  
	  
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Clear the background of our window to red  
	glClear(GL_COLOR_BUFFER_BIT); //Clear the colour buffer (more buffers later on)  
	glLoadIdentity(); // Load the Identity Matrix to reset our drawing locations  
	
	glTranslatef(0.0f, 0.0f, -5.0f); 
	glRotatef(xrot,1.0,0.0,0.0);  //rotate our camera on teh x-axis (left and right)
    glRotatef(yrot,0.0,1.0,0.0);  //rotate our camera on the y-axis (up and down)
	glTranslatef(-2.0f, -2.0f, 0.0f); 
	
	renderPrimitive(); // Render the primitive  
	  
	//glutWireCube(2.0f);

	glutSwapBuffers(); //swap the buffers
	//glFlush(); // Flush the OpenGL buffers to the window 
} 

void reshape (int width, int height) {  
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); // Set our viewport to the size of our window  
	glMatrixMode(GL_PROJECTION); // Switch to the projection matrix so that we can manipulate how our scene is viewed  
	glLoadIdentity(); // Reset the projection matrix to the identity matrix so that we don't get any artifacts (cleaning up)  
	gluPerspective(60, (GLfloat)width / (GLfloat)height, 1.0, 100.0); // Set the Field of view angle (in degrees), the aspect ratio of our window, and the new and far planes  
	glMatrixMode(GL_MODELVIEW); // Switch back to the model view matrix, so that we can start drawing shapes correctly  
}  

void keyOperations (void) {  
	if (keyStates['w']) { // If the left arrow key has been pressed  
		// Perform left arrow key operations  
		scale += 0.0001f;
	} 
	else if(keyStates['s']){
		scale -= 0.0001f;
	}
	else if(keyStates['l']){
	
		if(openFile())
			loadFile();

		keyStates['l'] = false;
	}
	else if(keyStates['q']){
		exit(0);
	}
	else if(keyStates['k']){
		showNotKalman = !showNotKalman;
		keyStates['k'] = false;
	}
	else if(keyStates['1']){
		//loadModelTrajectory(0);
		fitToModel(0);
		keyStates['1'] = false;
	}
	else if(keyStates['2']){
		loadModelTrajectory(1);
		keyStates['2'] = false;
	}
	else if(keyStates['3']){
		loadModelTrajectory(2);
		keyStates['3'] = false;
	}
	else if(keyStates['4']){
		loadModelTrajectory(3);
		keyStates['4'] = false;
	}
	else if(keyStates['5']){
		loadModelTrajectory(4);
		keyStates['5'] = false;
	}
}  

void mouseMovement(int x, int y) {
	int diffx=x-lastx; //check the difference between the current x and the last x position
	int diffy=y-lasty; //check the difference between the current y and the last y position
	lastx=x; //set lastx to the current x position
	lasty=y; //set lasty to the current y position
	xrot += (float) diffy; //set the xrot to xrot with the addition of the difference in the y position
	yrot += (float) diffx;// set the xrot to yrot with the addition of the difference in the x position
}

void mouseGo(int x, int y){
	lastx=x; //set lastx to the current x position
	lasty=y; //set lasty to the current y position

    return;
}


bool openFile(){
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Trajektoria\0*.trj\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn)==TRUE){
		return true;
	}
	return false;
}


bool loadFile(){
	
	ifstream file (szFile);

    if(file.is_open()) {
		trajectory.clear();
		trajectorySmooth.clear();

		int w, h, count, x, y, z;
		char c;
		file >> w;
		file >> h;
		file >> count;

		for(int i = 0; i < count && !file.eof(); ++i){
			file >> x >> c >> y >> c >> z;
			if(x != -1 && y != -1 && z!= -1){
				
				if(trajectory.size() >= 1){
					Point3D lastOne = trajectory.at(trajectory.size()-1);

					for(int k = 1; k < 5; ++k){
						trajectory.push_back(Point3D((k*x+(5-k)*lastOne.x)/5, 
														(k*y+(5-k)*lastOne.y)/5, 
														(k*z+(5-k)*lastOne.z)/5));
					}
				}
				trajectory.push_back(Point3D(x, y, z));
			}
		}

		file.close();
		return true;
    }
	else{
		return false;
	}
}

vector<Point3D> smoothTrajectory(vector<Point3D>& vect){

	vector<Point3D> toReturn;
	KalmanFilter KF(4, 2, 0);
    Mat_<float> state(4, 1); /* (x, y, Vx, Vy) */
    Mat processNoise(4, 1, CV_32F);
    Mat_<float> measurement(2,1); 
	
    char code = (char)-1;
	
	Point3D point = vect.at(0);

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

	for(unsigned int i = 1; i < vect.size() ;++i)
    {	
		Point3D point = vect.at(i);

        Mat prediction = KF.predict();
        Point predictPt(prediction.at<float>(0),prediction.at<float>(1));
		
        measurement(0) = point.x;
		measurement(1) = point.y;
		
		Point measPt(measurement(0),measurement(1));

		Mat estimated = KF.correct(measurement);
		toReturn.push_back(Point3D(estimated.at<float>(0),estimated.at<float>(1), point.z));
		//trajectorySmooth.push_back(Point3D(predictPt.x,predictPt.y, point.z));
	}
	return toReturn;
}


void circle(int cx, int cy, int radius){

	one.clear();

	for(int i = 0; i <= endangle; i+=2){
		float rad = i*PI/180.0f;
		one.push_back(Point3D(sin(rad)*radius+cx, cos(rad)*radius+cy, i));
	}

	int size = one.size();
	float zdiff = (float)(maxZ-100)/size;
	float z = 100;
	for(int i = 0; i < one.size(); ++i, z+=zdiff){
		Point3D curr = one[i];
		trajectoryModel.push_back(Point3D(curr.x, curr.y, (int)z));
	}
  
}

void frontArc(int begX, int begY, int radius, int maxY){

	one.clear();

	radius = (maxZ-100)/2;
	int xx = 100+radius;

	for(int i = 270; i <= 450; i+=2){
		float rad = i*PI/180.0f;
		one.push_back(Point3D(sin(rad)*radius+xx, cos(rad)*radius+begY, i));
	}

	int size = one.size();
	float zdiff = (float)(maxZ-100)/size;
	float z = 100;
	for(int i = 0; i < one.size(); ++i, z+=zdiff){
		Point3D curr = one[i];
		trajectoryModel.push_back(Point3D(xx, curr.y, curr.x));
	}
}

void fitToModel(int nr){
	
	trajectoryModel.clear();

	Point3D maxX, minX, maxY, minY, maxZ, minZ;
	minX = Point3D(1000, 1000, 1000);
	minY = Point3D(1000, 1000, 1000);
	minZ = Point3D(1000, 1000, 1000);

	int iMaxX, iMinX, iMaxY, iMinY, iMaxZ, iMinZ;
	int howMany = trajectorySmooth.size();

	lineXbegin = 0;
	lineYbegin = 0;
	// poczatkowy punkt z
	for(int i = 0; i < howMany/10; ++i){
		Point3D current = trajectorySmooth.at(i);
		if(current.z > maxZ.z){
			maxZ = current;
			iMaxZ = i;
		}
		lineXbegin += current.x;
		lineYbegin += current.y;
	}
	zbegin = maxZ.z;
	lineXbegin /= howMany/10;
	lineYbegin /= howMany/10;
	
	// koncowy punkt z
	for(int i = 9*howMany/10; i < howMany; ++i){
		Point3D current = trajectorySmooth.at(i);
		if(current.z < minZ.z){
			minZ = current;
			iMinZ = i;
		}
	}
	zend = minZ.z;


	// znajdujemy max i min x i y
	for(int i = 0; i < howMany; ++i){
		Point3D current = trajectorySmooth.at(i);
		
		if(current.x > maxX.x){
			maxX = current;
			iMaxX = i;
		}
		else if(current.x < minX.x){
			minX = current;
			iMinX = i;
		}

		if(current.y > maxY.y){
			maxY = current;
			iMaxY = i;
		}
		else if(current.y < minY.y){
			minY = current;
			iMinY = i;
		}
	}


	float diffZ = (float)(zend - zbegin)/(float)howMany;
	float diffX = (float)(maxX.x - lineXbegin)/(float)howMany;
	float diffY = (float)(maxY.y - lineYbegin)/(float)howMany;

	// linia prosta
	if(nr == LINE){
		float currZ = zbegin;
		for(int i = 0; i < howMany; ++i){
			trajectoryModel.push_back(Point3D(lineXbegin, lineYbegin, currZ));
			currZ+=diffZ;
		}

		float diffAvg = 0.0f;
		float diffAvg2 = 0.0f;
		for(int i = 0; i < howMany; ++i){
			diffAvg+=
			(abs(trajectorySmooth[i].x-trajectoryModel[i].x)+
			abs(trajectorySmooth[i].y-trajectoryModel[i].y)+
			abs(trajectorySmooth[i].z-trajectoryModel[i].z));

			diffAvg2+=
			(
			(trajectorySmooth[i].x-trajectoryModel[i].x)*(trajectorySmooth[i].x-trajectoryModel[i].x)+
			(trajectorySmooth[i].y-trajectoryModel[i].y)*(trajectorySmooth[i].y-trajectoryModel[i].y)+
			(trajectorySmooth[i].z-trajectoryModel[i].z)*(trajectorySmooth[i].z-trajectoryModel[i].z));
		}

		std::cout << "Roznica srednia bezwzgledna: " << diffAvg/howMany << std::endl;
		std::cout << "Roznica srednia kwadratowa: " << diffAvg2/howMany << std::endl;

		return;
	}

	// UWAGA! 

	// spirala
	if(nr == SPIRAL){
	/*
		one.clear();

		for(int i = 0; i <= endangle; i+=2){
			float rad = i*PI/180.0f;
			one.push_back(Point3D(sin(rad)*radius+lineXbegin, cos(rad)*radius+lineYbegin, i));
		}

		int size = one.size();
		float zdiff = (float)(maxZ-100)/size;
		float z = 100;
		for(int i = 0; i < one.size(); ++i, z+=zdiff){
			Point3D curr = one[i];
			trajectoryModel.push_back(Point3D(curr.x, curr.y, (int)z));
		}

	*/
		return;
	}
	

	// zygzak
	if(nr == ZIGZAG){
		float currX = lineXbegin;
		float currZ = zbegin;

		for(int i = 0; i < howMany/2; ++i){
			trajectoryModel.push_back(Point3D(currX, lineYbegin, currZ));
			currX+=diffX;
			currZ+=diffZ;
		}
		for(int i = howMany/2; i < howMany; ++i){
			trajectoryModel.push_back(Point3D(currX, lineYbegin, currZ));
			currX-=diffX;
			currZ+=diffZ;
		}
		return;
	}
	
	// up arc
	if(nr == UP){
	
		one.clear();

		int radius = (zend - zbegin)/2;
		int xx = 100+radius;
		float stDiff = (450-270)/(howMany);
		float currSt = 270;

		for(int i = 0; i <= howMany; ++i){
			float rad = currSt*PI/180.0f;
			one.push_back(Point3D(sin(rad)*radius+zbegin, cos(rad)*radius+maxZ.y, currSt));
			currSt += stDiff;
		}

		for(int i = 0; i < howMany; ++i){
			Point3D curr = one[i];
			trajectoryModel.push_back(Point3D(lineXbegin, curr.y, curr.x));
		}

		return;
	}
}


void loadModelTrajectory(int model){

	trajectoryModel.clear();

	switch(model){
		case 0:
			// linia prosta
			for(int i = zbegin; i < zend; ++i){
				trajectoryModel.push_back(Point3D(lineXbegin, lineYbegin, i));
			}

			break;

		case 1:
			// spirala
			circle(lineXbegin, lineYbegin, circleRadius);

			break;

		case 2:
			// zygzak
			for(int i = 100; i < (maxZ-100)/2+100; ++i){
				trajectoryModel.push_back(Point3D(lineXbegin++, lineYbegin, i));
			}
			for(int i = (maxZ-100)/2+100; i < maxZ; ++i){
				trajectoryModel.push_back(Point3D(lineXbegin--, lineYbegin, i));
			}
			break;

		case 3:
			// up
			//frontArc(lineXbegin, lineYbegin, circleRadius, maxY);
			break;
	}
}