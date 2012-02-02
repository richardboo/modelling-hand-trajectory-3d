#include <vector>
#include <windows.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <opencv2/video/tracking.hpp>
#include <cv.h>
#include <math.h>
#include <stdio.h>
#include <iostream>

#include "trajectorysample.h"
#include "point3d.h"
#include "modeltrajectory.h"
#include "modeltrajectoryfactory.h"
#include "recognitionmodule.h"

#define PI 3.14
#define ORIGINAL 0
#define TRAJECTORIES 1
#define FITTED 2

using namespace std;

// zmienne pomocnicze
float xpos = 0, ypos = 0, zpos = 0, xrot = 0, yrot = 0, angle=0.0;
float lastx, lasty;
float scale = 0.008f;
bool keyStates[256];
OPENFILENAME ofn;       // common dialog box structure
char szFile[260];       // buffer for file name
HWND hwnd = NULL;       // owner window
HANDLE hf;              // file handle

int currentSampleIndex = -1;

// dane do badania
vector<TrajectorySample *> samples;
ModelTrajectoryFactory trajFactory;

// co wyswietlac? original/trajektories/fitted
int viewState = 0;
// ktora modelowa trajektorie?
int modelTrajektoryNr = 0;
// ktora trajektoria z badanych do pokazania? (1,2,4,8,16) (z-b)
int sampleToShow = 0;
// ktora trajektoria badana? 0-2
int testedTrajectory = 0;



// opengl functions
void keyUp (unsigned char key, int x, int y);
void keyPressed (unsigned char key, int x, int y);
void reshape (int width, int height);
void display ();
void keyOperations ();
void renderPrimitive ();
void renderStrings();
void mouseMovement(int x, int y);
void mouseGo(int, int);


bool loadFile();
bool openFile();
void showHelp();
void saveCurrentStat();
void saveAllStat();
void drawTrajectoryWithColor(float r, float g, float b, vector<Point3D> & points);


int main (int argc, char **argv) {  
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

	trajFactory.generateOriginalTrajectories(640, 480, 300);
	showHelp();
	  
	glutMainLoop(); // Enter GLUT's main loop  
}


/////////////////////////////// LADOWANIE NOWEJ PROBKI ///////////////////////////
bool loadFile(){
	ifstream file (szFile);

    if(file.is_open()) {
		TrajectorySample * newSample = new TrajectorySample;
		newSample->loadSample(file);
		file.close();

		samples.push_back(newSample);
		currentSampleIndex = samples.size()-1;

		newSample->prepareAllTrajectories();
		trajFactory.fitModelTrajectories(newSample);
		newSample->recognize();
		newSample->showStats();
		newSample->isReady = true;

		return true;
    }
	return false;
}


/////////////////////////////// WYSWIETLANIE /////////////////////////////////////
void renderPrimitive () {

	
	glPushMatrix();
	glScalef(scale,scale,-scale);
	

	// SCENA //
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

	if(viewState == ORIGINAL){
		// zalezne od modelTrajektoryNr
		drawTrajectoryWithColor(1.0f, 0.0f, 0.0f, trajFactory.originalTrajectories[modelTrajektoryNr]->points);
	}
	else if(viewState == TRAJECTORIES){
		if(currentSampleIndex > -1 && (int)samples.size() < currentSampleIndex){
			TrajectorySample * sample = samples[currentSampleIndex];
			if(!sample->isReady){
				return;
			}
			if(sampleToShow & 1){
				drawTrajectoryWithColor(1.0f, 1.0f, 0.0f, sample->originalPoints);
			}
			if(sampleToShow & 2){
				drawTrajectoryWithColor(0.0f, 1.0f, 1.0f, sample->filteredTrajectory.points);
			}
			if(sampleToShow & 4){
				drawTrajectoryWithColor(0.0f, 0.0f, 1.0f, sample->kalmanTrajectory.points);
			}
			if(sampleToShow & 8){
				drawTrajectoryWithColor(1.0f, 1.0f, 1.0f, sample->kalman3Trajectory.points);
			}
			if(sampleToShow & 16){
				drawTrajectoryWithColor(0.0f, 1.0f, 1.0f, sample->kalmanTrajectory.fittedModels[sample->kalmanTrajectory.recognizedIndex]->points);
			}
		}
	}
	else if(viewState == FITTED){
		//testedTrajectory+modelTrajectoryNr
		if(currentSampleIndex > -1 && (int)samples.size() < currentSampleIndex){
			TrajectorySample * sample = samples[currentSampleIndex];
			if(!sample->isReady){
				return;
			}
			if(testedTrajectory == 1){
				drawTrajectoryWithColor(1.0f, 1.0f, 0.0f, sample->filteredTrajectory.points);
				drawTrajectoryWithColor(1.0f, 0.0f, 0.0f, sample->filteredTrajectory.fittedModels[modelTrajektoryNr]->points);
			}
			else if(testedTrajectory == 2){
				drawTrajectoryWithColor(0.0f, 1.0f, 1.0f, sample->kalmanTrajectory.points);
				drawTrajectoryWithColor(1.0f, 0.0f, 0.0f, sample->kalmanTrajectory.fittedModels[modelTrajektoryNr]->points);
			}
			else if(testedTrajectory == 3){
				drawTrajectoryWithColor(0.0f, 0.0f, 1.0f, sample->kalman3Trajectory.points);
				drawTrajectoryWithColor(1.0f, 0.0f, 0.0f, sample->kalman3Trajectory.fittedModels[modelTrajektoryNr]->points);
			}
		}
	}

	glPopMatrix();
}

void renderStrings(){
	// tryb wysiwetlania
	glRasterPos2i(30, 610);
	glColor3f(1.0f, 1.0f, 1.0f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)(viewState == 0? "modelowe trajektorie": (viewState == 1 ? "badane trajektorie" : "dopasowane trajektorie")));

	// ktora trajektoria modelowa?
	int nextY = 590;
	if(viewState == ORIGINAL || viewState == FITTED){
		glRasterPos2i(30, nextY);
		glColor3f(0.8f, 0.2f, 0.2f);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)"model: ");
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)ModelTrajectory::name[modelTrajektoryNr]);
		nextY -= 20;
	}
	if(viewState == TRAJECTORIES){
		glRasterPos2i(30, nextY);
		glColor3f(0.8f, 0.2f, 0.2f);
		nextY -= 20;
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)"badane: ");
		glRasterPos2i(30, nextY);
		if(sampleToShow & 1){
			glColor3f(1.0f, 1.0f, 0.0f);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)TrajectorySample::trajName[0]);
		}
		if(sampleToShow & 2){
			glColor3f(0.0f, 1.0f, 1.0f);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)TrajectorySample::trajName[1]);
		}
		if(sampleToShow & 4){
			glColor3f(0.0f, 0.0f, 1.0f);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)TrajectorySample::trajName[2]);
		}
		if(sampleToShow & 8){
			glColor3f(1.0f, 1.0f, 1.0f);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)TrajectorySample::trajName[3]);
		}
		if(sampleToShow & 16){
			glColor3f(0.0f, 1.0f, 1.0f);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)TrajectorySample::trajName[4]);
		}
		nextY -=20;
	}
	if(viewState == FITTED){
		glRasterPos2i(30, nextY);
		glColor3f(0.2f, 0.8f, 0.2f);
		nextY -= 20;
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)"badana: ");
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)TrajectorySample::trajName[testedTrajectory]);
	}
}

///////////////////////////////// WYBOR AKCJI /////////////////////////////////
// w, s - sterowanie kamera
// l - load nowej trajektorii
// q - zakonczenie programu
//
// o - zmiana on/off pokazywania orginalej trajektorii i probki
// 1-5 - wybor trajektorii modelowej, zaleznie od o pokazuje odpowiednia (1-5)
// z - orginalna trajektoria		(1)
// x - trajektoria po probkowaniu	(2)
// c - po filtrze kalmana			(4)
// v - po 3x filtrze kalmana		(8)
// b - dopasowana trajektoria		(16)
//
// m - zapis statystyk
// n - zapis wszystkich statystyk
//
// h - pomoc
void keyOperations (void) {

	if (keyStates['w']) { // If the left arrow key has been pressed
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
	}else if(keyStates['o']){
		viewState = (viewState+1)%3;
		keyStates['o'] = false;
	}
	else if(keyStates['m']){
		saveCurrentStat();
		keyStates['m'] = false;
	}
	else if(keyStates['h']){
		showHelp();
		keyStates['h'] = false;
	}
	else if(keyStates['1']){
		modelTrajektoryNr = 0;
		keyStates['1'] = false;
	}else if(keyStates['2']){
		modelTrajektoryNr = 1;
		keyStates['2'] = false;
	}else if(keyStates['3']){
		modelTrajektoryNr = 2;
		keyStates['3'] = false;
	}else if(keyStates['4']){
		modelTrajektoryNr = 3;
		keyStates['4'] = false;
	}else if(keyStates['5']){
		modelTrajektoryNr = 4;
		keyStates['5'] = false;
	}
	else if(viewState == TRAJECTORIES){
		if(keyStates['z']){
			if(sampleToShow & 1)	sampleToShow -= 1;
			else					sampleToShow += 1;
			keyStates['z'] = false;
		}else if(keyStates['x']){
			if(sampleToShow & 2)	sampleToShow -= 2;
			else					sampleToShow += 2;
			keyStates['x'] = false;
		}else if(keyStates['c']){
			if(sampleToShow & 4)	sampleToShow -= 4;
			else					sampleToShow += 4;
			keyStates['c'] = false;
		}else if(keyStates['v']){
			if(sampleToShow & 8)	sampleToShow -= 8;
			else					sampleToShow += 8;
			keyStates['v'] = false;
		}else if(keyStates['b']){
			if(sampleToShow & 16)	sampleToShow -= 16;
			else					sampleToShow += 16;
			keyStates['b'] = false;
		}
	}
	else if(viewState == FITTED){
		if(keyStates['z']){
			testedTrajectory = 1;
			keyStates['z'] = false;
		}else if(keyStates['x']){
			testedTrajectory = 2;
			keyStates['x'] = false;
		}else if(keyStates['c']){
			testedTrajectory = 3;
			keyStates['c'] = false;
		}
	}

}


/////////////////////////////// POMOC /////////////////////////////
void showHelp(){
	cout << "***** POMOC *****"<<endl<<endl;
	cout << "** l - wczytanie trajektorii"<<endl;
	cout << "** h - pomoc"<<endl;
	cout << "** q - zakonczenie"<<endl;
	cout << "** w/s - sterowanie kamera"<<endl;
	cout << endl;
	cout << "** o - zmiana trybu wyswietlania:"<<endl;
	cout << "**\tORIGINAL - oryginalne trajektorie mdoelowe"<<endl;
	cout << "**\tTRAJECOTORIES - badane trajektorie"<<endl;
	cout << "**\tFITTED - dopasowane trajektorie modelowe"<<endl;
	cout << endl << "\tORIGINAL:" << endl;
	cout << "zmiana trajektorii: 1-5" << endl<<endl;
	cout << "\tTRAJECTORIES:"<<endl;
	cout << "wyswietlanie trajektorii on/off: z-b"<<endl<<endl;
	cout << "\tFITTED:"<<endl;
	cout << "wyswietlanie trajektorii: z-c"<<endl;
	cout << "modelowa dopasowana do niej: 1-5"<<endl;
}


/////////////////////////////// ZAPIS ////////////////////////////
void saveCurrentStat(){
	
}

void saveAllStat(){

}


/////////////////////////////// FUNCKJE OPENGL ////////////////////
void drawTrajectoryWithColor(float r, float g, float b, vector<Point3D> & trajectory){
	glBegin(GL_LINES);
	glPointSize(2.0f);  
	glColor3f(r, g, b);
	
	if(trajectory.size() > 0){
		for(unsigned int i = 0; i < trajectory.size()-1; i++){
			glVertex3f(trajectory[i].x, trajectory[i].y, trajectory[i].z);
			glVertex3f(trajectory[i+1].x, trajectory[i+1].y, trajectory[i+1].z);
		}
	}
	
	glEnd();
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
	

    glMatrixMode(GL_PROJECTION);   //select the Projection matrix
    glPushMatrix();                   //save the current projection matrix
    glLoadIdentity();                 
    glOrtho(0, 640, 0, 640, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

	renderStrings();

	glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

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

void mouseMovement(int x, int y) {
	float diffx= x-lastx; //check the difference between the current x and the last x position
	float diffy= y-lasty; //check the difference between the current y and the last y position
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

