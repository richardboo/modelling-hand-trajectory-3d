#include "mgrstereohand.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QByteArray>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDebug>

#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <omp.h>
#include <time.h>

#include "settings.h"
#include "imageutils.hpp"


static IplImage * frame[2], * frameSmaller[2], *frameShow[2];
static IplImage * frameBackground[2];
static IplImage * frameDiff[2];
static IplImage * frameRectified[2];
static IplImage * frameGray[2];
static IplImage * frameSmallerGray[2];
static IplImage * frameSkin[2];
static IplImage * frameBlob[2];

static IplImage * blackImage;

static IplImage * disparity = NULL;
static IplImage * disparitySmaller = NULL;
static IplImage * disparityToShow = NULL;
static IplImage * trajectory = NULL;
static IplImage * trajectorySmaller = NULL;

float ALPHA_BG = 0.07f;
static int BG_UPDATE_FRAMES = 35;
int currentBgFrames = 0;
int FPS = 15;

int backgroundThreshold = 15;

int segmantationAlg = 0;
int stereoAlg = 0;

float lastUpdate;
float fpsUpdateInterval;
int  numFrames;
float timeAtGameStart;

time_t startTime = 0;
time_t endTime = 0;

float avgRectifyTime = 0;
float avgFindSkinTime = 0;
float avgFindHandTime = 0;
float avgStereoTime = 0;

time_t beginActionTime = 0;
time_t endActionTime = 0;

long framesCounter;
float fps;
int bps;
long allBits;



using namespace std;
using namespace cv;


MgrStereoHand::MgrStereoHand(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	initUI();

	lastUpdate        = 0;
	fpsUpdateInterval = 0.5f;
	numFrames         = 0;
	fps               = 0;
	bps				  = 0;
	allBits			  = 0;

	calibration = false;
	nothing = true;

	fileFilm0 = "";
	fileFilm1 = "";
	fileCalib = "";

	frameGrabber[0] = frameGrabber[1] = NULL;
	//videoWriter[0] = videoWriter[1] = NULL;

	//buffer[0] = vector<IplImage *>(800);
	//buffer[1] = vector<IplImage *>(800);
	srModule = new SignRecognitionModule("A.xml");
	startRecognized[0] = startRecognized[1] = 0;
	lastStart[0] = lastStart[1] = false;

	hand[0] = hand[1] = head[0] = head[0] = NULL;
}

MgrStereoHand::~MgrStereoHand(){
	delete leftCamWindow;
	delete rightCamWindow;
	delete disparityWindow;
	delete trajectoryWindow;

	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		delete frameGrabber[1];
	}

	if(frameSmaller[0] != NULL){
		cvReleaseImage(&frameSmaller[0]);
		cvReleaseImage(&frameSmaller[1]);
	}

	
	if(disparity != NULL){
		for(int i = 0; i < 2; ++i){
			cvReleaseImage(&frameSmaller[i]);
			cvReleaseImage(&frameSmallerGray[i]);
			cvReleaseImage(&frameShow[i]);
			cvReleaseImage(&frameDiff[i]);
			cvReleaseImage(&frameBackground[i]);
			cvReleaseImage(&frameRectified[i]);
			cvReleaseImage(&frameGray[i]);
			cvReleaseImage(&frameSkin[i]);
			cvReleaseImage(&frameBlob[i]);

			if(hand[i] != NULL)
				delete hand[i];
			if(head[i] != NULL)
				delete head[i];
		}

		cvReleaseImage(&blackImage);
		cvReleaseImage(&disparity);
		cvReleaseImage(&disparitySmaller);
		cvReleaseImage(&disparityToShow);
		cvReleaseImage(&trajectory);
		cvReleaseImage(&trajectorySmaller);
	}

	delete drawingModule;
	delete calibModule;
	delete srModule;
}

void MgrStereoHand::mainIdleLoop(){
	
	// tylko jesli sa kamery
	if(frameGrabber[0] != NULL && frameGrabber[1] != NULL){

		for(int i = 0; i < 2; ++i){
			if(!frameGrabber[i]->hasNextFrame()){
				// TODO jesli jest video to zatrzymanie
				// i zebranie statystyk
				
				return;
			}
			frame[i] = frameGrabber[i]->getNextFrame();
		}

		cvResize(frame[0], frameSmaller[0]);//, CV_INTER_NN);
		cvResize(frame[1], frameSmaller[1]);//, CV_INTER_NN);
		leftCamWindow->showImage(frameSmaller[0]);
		rightCamWindow->showImage(frameSmaller[1]);
		trajectoryWindow->showImage(blackImage);
		disparityWindow->showImage(blackImage);
	}
	else{
		leftCamWindow->showImage(blackImage);
		rightCamWindow->showImage(blackImage);
		trajectoryWindow->showImage(blackImage);
		disparityWindow->showImage(blackImage);
	}

	if(!nothing){

		if(!checkIfStart()){
			qDebug() << "No trudno";
			nothing = true;
			return;
		}

		framesTimer->stop();
		delete framesTimer;
		makeEverythingStart();
	}
}

//
// 1. kalibracja
// 2. przetwarzanie zwykle, bez niczego - rozpoznawanie tylko dloni, w petli mainIdleLoop
// 3. przetwarzanie na zywo - wszystko, buforowanie, mozna zapisac po
// 4. odtwarzanie z filmow - wszystko, do momentu skonczenia pobierania klatek
void MgrStereoHand::mainLoop(){

	try{
	for(int i = 0; i < 2; ++i){
		if(!frameGrabber[i]->hasNextFrame()){
			// TODO jesli jest video to zatrzymanie
			// i zebranie statystyk

			if(processType == VIDEO){
				framesTimer->stop();
				delete framesTimer;

				makeEverythingStop();
			}
			
			return;
		}
		frame[i] = frameGrabber[i]->getNextFrame();
	}

	// wyliczenie fps
	framesCounter++;
	updateFPS();

	CvRect rect =cvRect(0, 
						0, 
						Settings::instance()->defSize.width,
						Settings::instance()->defSize.height);

	if(!calibration){
		// no to przetwarzamy wszystko

		if(processType == CAMERA){
			IplImage * copy = cvCreateImage(Settings::instance()->defSize, 8, 3);
			cvCopyImage(frame[0], copy);
			buffer[0].push_back(copy);

			IplImage * copy2 = cvCreateImage(Settings::instance()->defSize, 8, 3);
			cvCopyImage(frame[1], copy2);
			buffer[1].push_back(copy2);
		}
		

		int i;
		//#pragma omp parallel for shared(frameRectified,frameShow,frame,frameSkin,segmantationAlg,frameDiff) private(i)
		for(i = 0; i < 2; ++i){

			//qDebug() << "Hello from thread " << omp_get_thread_num();

			// jakbym chciala liczyc czas
			//time(&beginActionTime);
			calibModule->rectifyImage(frame[i], frameRectified[i], i);
			//time(&endActionTime);
			//avgRectifyTime += difftime(beginActionTime, beginActionTime);

			cvCopyImage(frame[i], frameShow[i]);
		}

		if((segmantationAlg == HIST_ || segmantationAlg == ALL_) && stateHist != STATE_AFTER_HIST){
			for(int i = 0; i < 2; ++i){
				
				// pobieranie histogramu skory
				// pierwsze sprawdzamy to, czy juz jest to zrobione, aby ograniczyc wywolanie porownan
				
				if(faceDetection[i]->findHeadHaar(frameRectified[i])){
					// jak znaleziono - update histogramu
					skinDetection[i]->updateHistogram(frameRectified[i], faceDetection[i]->head.getSmallerRect());
					// no i narysujmy ja
					cvRectangleR(frameShow[i], faceDetection[i]->head.lastRect, cvScalar(0, 0, 255), 2);
				}
				drawingModule->drawSamplesOnFrame(skinDetection[i]->getSampleCount(), frameShow[i]);

				// jak juz jest wystarczajaco duzo probek - koniec
				if(	skinDetection[i]->getSampleCount() >= 20){
					skinDetection[i]->skinFound = true;

					// ustawienie wskaznika na twarz, aby mozna bylo znalezc dlon
					head[i] = &faceDetection[i]->head;
					// dawne
					// handDetection[i]->head = &faceDetection[i]->head;
				}

				// po przejsciu calej petli - jesli mamy juz w obydwu znaleziona skore -konczymy
				if(skinDetection[0]->skinFound && skinDetection[1]->skinFound){
					stateHist = STATE_AFTER_HIST;
				}
			}
		}
		else if(segmantationAlg == ONLY_BG_ || segmantationAlg == BGSKIN_){
			
			if(stateBack == STATE_BEFORE_BACK){
				for(int i = 0; i < 2; ++i){
					
					cvCvtColor(frameRectified[i], frameGray[i], CV_BGR2GRAY);
					cvAddWeighted(frameGray[i], ALPHA_BG, frameBackground[i], (1.0-ALPHA_BG), 0.0, frameBackground[i]);
					if(faceDetection[i]->lastFound.x == -1){
						faceDetection[i]->findHeadHaar(frameRectified[i]);
					}
				}
				currentBgFrames++;

				// wylaczamy dopiero jak chociaz raz byla rozpoznana twarz
				if(currentBgFrames >= BG_UPDATE_FRAMES && 
					faceDetection[0]->lastFound.x > -1 &&
					faceDetection[1]->lastFound.x > -1){

					stateBack = STATE_AFTER_BACK;
					cvSetImageROI(frameBackground[0], faceDetection[0]->lastFound);
					cvSetImageROI(frameBackground[1], faceDetection[1]->lastFound);
					cvSet(frameBackground[0], cvScalarAll(0));
					cvSet(frameBackground[1], cvScalarAll(0));
					cvResetImageROI(frameBackground[0]);
					cvResetImageROI(frameBackground[1]);
				}
			}
			else{

				#pragma omp parallel for default (shared) private(i)
				for(int i = 0; i < 2; ++i){
					cvCvtColor(frameRectified[i], frameGray[i], CV_BGR2GRAY);
					if(framesCounter % 50 == 0)
						cvAddWeighted(frameGray[i], ALPHA_BG, frameBackground[i], (1.0-ALPHA_BG), 0.0, frameBackground[i]);
					cvAbsDiff(frameGray[i], frameBackground[i], frameDiff[i]);
					cvThreshold(frameDiff[i], frameDiff[i], backgroundThreshold, 255, CV_THRESH_BINARY);
				}
			}
			//cvShowImage("gray", frameDiff[0]);
			//cvShowImage("diff", frameDiff[0]);
		}

		int handFound[2];
		handFound[0] = 3;
		handFound[1] = 3;
		// dawne
		// bool handFound[2];
		// handFound[0] = false;
		// handFound[1] = false;
		bool changeStart = false;
		//#pragma omp parallel for shared(changeStart, frameRectified,frameShow,frame,frameBlob,frameSkin,segmantationAlg,frameDiff) private(i)
		for(int i = 0; i < 2; ++i){
			skinDetection[i]->detectSkin(frameRectified[i], frameSkin[i], rect, segmantationAlg, frameDiff[i]);
		
			// dawne
			//handFound[i] = handDetection[i]->findHand(frameSkin[i], frameBlob[i], rect, *hand[i]);
			//if(handFound[i]){
		/*
			cvCvtColor(frameRectified[i], frameGray[i], CV_BGR2GRAY);
			cvZero(frameGray[1]);
			cvCopy(frameGray[i], frameGray[1], frameSkin[i]);
			
			IplImage * contour = cvCreateImage(cvGetSize(frameSkin[i]), IPL_DEPTH_8U, 1);
			cvCanny( frameGray[1], contour, 10, 200, 5);
			cvDilate(contour, contour);

			IplImage * contourColor = cvCreateImage(cvGetSize(frameSkin[i]), IPL_DEPTH_8U, 3);
			cvCvtColor(contour, contourColor, CV_GRAY2BGR);
			cvAdd(frameRectified[i], contourColor, contourColor, frameSkin[i]);

			cvFloodFill(contourColor, cvPoint(200, 200), cvScalarAll(255), cvScalarAll(40), cvScalarAll(40));

			cvShowImage("contour", contourColor);

			*/
			//cvCvtColor(frameRectified[i], frameGray[i], CV_BGR2GRAY);

			handFound[i] = handDetection[i]->findHand(frameSkin[i], frameBlob[i], frame[i], rect, *hand[i], *head[i]);
			
			if(!handFound[i]){
				bool start = srModule->isSign(frameBlob[i], hand[i]->lastRect);
				if(!start && Settings::instance()->changeTrajectory){
					Settings::instance()->changeTrajectory = false;
					start = true;
				}
				changeStart = (start != lastStart[i]);
				lastStart[i] = start;
			}
			else{
			}
		
		}

		if(changeStart && lastStart[0] == lastStart[1]){
			startRecognized[0] = (startRecognized[0]+1);
		}


		qDebug() << "i: " << startRecognized[0] << " " << startRecognized[1];

		// ONE
		
		if(!handFound[0] && !handFound[1] && (startRecognized[0] == 2)){

			displayOverlay(leftCamWindow->name, "START RECOGNIZED", 1000);

			if(stereoAlg < MINE_)
				stereoModule->stereoProcessGray(frameGray, frameBlob, hand, disparity, stereoAlg);
			else
				stereoModule->stereoProcessMine(frameGray, frameBlob, hand, disparity, stereoAlg);
			
			cvResize(disparity, disparitySmaller, CV_INTER_NN);
			drawingModule->drawDispOnFrame(hand[0]->lastZ, disparitySmaller, disparityToShow);
			disparityWindow->showImage(disparityToShow);
		}else{
			hand[0]->setLastPointWithZ(-1);
			hand[1]->setLastPointWithZ(-1);

			cvZero(disparityToShow);
			disparityWindow->showImage(disparityToShow);
		}

		drawingModule->drawTrajectoryOnFrame(hand[0], trajectorySmaller);
		trajectoryWindow->showImage(trajectorySmaller);
		
	}
	else{
	// KALIBRACJA

		// kalibracja, bierzemy co 20 klatke
		if(framesCounter % 10 == 0){

			// migniecie
			leftCamWindow->showImage(blackImage);
			rightCamWindow->showImage(blackImage);

			// kalibracja
			ImageUtils::getGray(frame[0], frameGray[0]);
			ImageUtils::getGray(frame[1], frameGray[1]);

			calibModule->calibrationAddSample(frameGray[0], frameGray[1]);
			
			if(calibModule->getSampleCount() == calibModule->getMaxSamples()){

				framesTimer->stop();
				calibModule->calibrationEnd();
				calibDialog->endCalibration();
				stopProcess();
			}
			else{
				return;
			}
		}
		cvCopyImage(frame[0], frameShow[0]);
		cvCopyImage(frame[1], frameShow[1]);
	}

	//cvCopyImage(frame[0], frameShow[0]);
	//cvCopyImage(frame[1], frameShow[1]);
	showImages();

	// koniec petli
	if(nothing){
		// zatrzymujemy
		framesTimer->stop();
		delete framesTimer;
		time(&endTime);
		makeEverythingStop();
		return;
	}
	}catch(cv::Exception ex){
		
	}
}


void MgrStereoHand::showImages(){

	switch(Settings::instance()->imageType){
		case 0:

			cvResize(frameShow[0], frameSmaller[0]);//, CV_INTER_NN);
			cvResize(frameShow[1], frameSmaller[1]);//, CV_INTER_NN);
			drawingModule->drawFPSonFrame(fps > 0 ? fps : 0, frameSmaller[0]);
			leftCamWindow->showImage(frameSmaller[0]);
			rightCamWindow->showImage(frameSmaller[1]);
			break;
		case 1:

			cvResize(frameRectified[0], frameSmaller[0], CV_INTER_NN);
			cvResize(frameRectified[1], frameSmaller[1], CV_INTER_NN);
			drawingModule->drawFPSonFrame(fps > 0 ? fps : 0, frameSmaller[0]);

			for(int i = 0; i < 2; ++i){
				if(hand[i]->lastRect.x > -1 ){
					drawingModule->drawSmallerRectOnFrame(hand[i]->lastRect, frameSmaller[i], cvScalar(200,0,0));
					//drawingModule->drawSmallerRectOnFrame(head[0]->lastRect, frameSmaller[0], cvScalar(0,200,0));
				}
				if(head[i]->lastRect.x > -1){
					drawingModule->drawSmallerRectOnFrame(head[i]->lastRect, frameSmaller[i], cvScalar(0,200,0));
				}
			}
			leftCamWindow->showImage(frameSmaller[0]);
			rightCamWindow->showImage(frameSmaller[1]);
			break;
		case 2:
			cvResize(frameSkin[0], frameSmallerGray[0], CV_INTER_NN);
			cvResize(frameSkin[1], frameSmallerGray[1], CV_INTER_NN);
			drawingModule->drawFPSonFrame(fps > 0 ? fps : 0, frameSmaller[0]);
			leftCamWindow->showImage(frameSmallerGray[0]);
			rightCamWindow->showImage(frameSmallerGray[1]);
			break;
		case 3:
			
			if(hand[0]->lastRect.x != -1){
				drawingModule->drawTextOnFrame("znaleziono", frameBlob[0]);
				drawingModule->drawMiddleOnFrame(hand[0], frameBlob[0]);
			}
			if(hand[1]->lastRect.x != -1){
				drawingModule->drawTextOnFrame("znaleziono", frameBlob[1]);
				drawingModule->drawMiddleOnFrame(hand[1], frameBlob[1]);
			}

			cvResize(frameBlob[0], frameSmallerGray[0], CV_INTER_NN);
			cvResize(frameBlob[1], frameSmallerGray[1], CV_INTER_NN);
			drawingModule->drawFPSonFrame(fps, frameSmallerGray[0]);
			leftCamWindow->showImage(frameSmallerGray[0]);
			rightCamWindow->showImage(frameSmallerGray[1]);
			
			break;
	}

}


// inicjalizuje wszystko
// tworzy okna
// tworzy frames'y
// tworzy camery
bool MgrStereoHand::init(){
	initWindows();
	initFrames();
	initCameras();

	for(int i = 0; i < 2; ++i){
		// usuwanie tla
		bgmask[i] = NULL;

		// wykrywanie twarzy
		faceDetection[i] = NULL;

		// wykrywanie skory
		skinDetection[i] = NULL;

		// sledzenie dloni
		handDetection[i] = NULL;
	}

	stereoModule = NULL;
	drawingModule = new DrawingModule();
	calibModule = new CalibrationModule();
	calibDialog->init(calibModule);


	framesTimer = new QTimer(this);
	connect(framesTimer, SIGNAL(timeout()), this, SLOT(mainIdleLoop()));
	framesTimer->start(1);

	return true;
}


bool MgrStereoHand::initWindows(){
	leftCamWindow = new MyWindow("kamera 1");
	leftCamWindow->setXY(350, 180);

	rightCamWindow = new MyWindow("kamera 2");
	rightCamWindow->setXY(670, 180);
	
	disparityWindow = new MyWindow("mapa glebokosci");
	disparityWindow->setXY(350, 490);

	trajectoryWindow = new MyWindow("trajektoria");
	trajectoryWindow->setXY(670, 490);

	return true;
}

// reinicjalizacja zawsze zachodzi z mainIdleLoop
// sa juz zainicjalizowane kamery
// inicjalizujemy filmy tylko wtedy, kiedy bedzie VIDEO
bool MgrStereoHand::reinitAll(){

	bool all = true;
	if(processType == VIDEO){
		all = initFilms();
	}

	startRecognized[0] = startRecognized[1] = 0;
	lastStart[0] = lastStart[1] = false;
	Settings::instance()->changeTrajectory = false;

	reinitFrames();

	if(!calibration){
		reinitModules();
		hand[0] = new Blob();
		hand[1] = new Blob();
		head[0] = new Blob();
		head[1] = new Blob();
	}

	stateHist = STATE_BEFORE_HIST;
	stateBack = STATE_BEFORE_BACK;

	framesCounter = 0;
	fps = 0;
	initGameTime();
	allBits = 0;
	bps = 0;
	numFrames = 0;
	currentBgFrames = 0;

	stereoAlg = Settings::instance()->stereoAlg;
	segmantationAlg = Settings::instance()->segmantationAlg;

	return all;
}

bool MgrStereoHand::initCameras(){

	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		delete frameGrabber[1];
	}

	CameraDevice * cam1 = new CameraDevice(0);
	//CameraDevice * cam2 = cam1;
	CameraDevice * cam2 = new CameraDevice(1);

	// ONE
	//cam1->init(Settings::instance()->defSize.width,
	//			   Settings::instance()->defSize.height);


	if(!cam1->init(Settings::instance()->defSize.width,
				   Settings::instance()->defSize.height) || 
	   !cam2->init(Settings::instance()->defSize.width,
	   Settings::instance()->defSize.height)){
		frameGrabber[0] = NULL;
		frameGrabber[1] = NULL;
		return false;
	}

	frameGrabber[0] = cam1;
	frameGrabber[1] = cam2;

	cam1->hasNextFrame();
	cam2->hasNextFrame();

	frame[0] = cam1->getNextFrame();
	frame[1] = cam2->getNextFrame();

	return true;
}

bool MgrStereoHand::initFilms(){
	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		delete frameGrabber[1];
	}

	VideoGrabber * film1 = new VideoGrabber(fileFilm0);
	VideoGrabber * film2 = new VideoGrabber(fileFilm1);

	if(!film1->init() || !film2->init())
		return false;

	frameGrabber[0] = film1;
	frameGrabber[1] = film2;

	film1->hasNextFrame();
	film2->hasNextFrame();

	frame[0] = film1->getNextFrame();
	frame[1] = film2->getNextFrame();

	CvSize size = cvGetSize(frame[0]);
	return true;
}

/**
	Reinicjalizacja modulow przed rozpoczeciem przetwarzania.
	Wykorzystuje m.in. rozmiar przetwarzania
*/
bool MgrStereoHand::reinitModules(){

	if(bgmask[0] != NULL){
		for(int i = 0; i < 2; ++i){
			delete bgmask[i];
			delete faceDetection[i];
			delete skinDetection[i];
			delete handDetection[i];
		}
	}
	if(stereoModule != NULL){
		delete stereoModule;
		stereoModule = NULL;
	}

	for(int i = 0; i < 2; ++i){
		// usuwanie tla
		bgmask[i] = new BGMask;
		bgmask[i]->init(false, frame[i]);

		// wykrywanie twarzy
		faceDetection[i] = new FaceDetector;
		if(!faceDetection[i]->init())
			return false;

		// wykrywanie skory
		skinDetection[i] = new SkinDetector;

		// sledzenie dloni
		handDetection[i] = new HandDetector;
		handDetection[i]->init(Settings::instance()->defSize);
	}
	
	stereoModule = new StereoModule();
	stereoModule->stereoStart(Settings::instance()->defSize);
	return true;
}


void MgrStereoHand::reinitFrames(){

	if(disparity != NULL){
		for(int i = 0; i < 2; ++i){
			cvZero(frameSmaller[i]);
			cvZero(frameSmallerGray[i]);
			cvZero(frameShow[i]);
			cvZero(frameDiff[i]);
			cvZero(frameBackground[i]);
			cvZero(frameRectified[i]);
			cvZero(frameGray[i]);
			cvZero(frameSkin[i]);
			cvZero(frameBlob[i]);
		}

		cvZero(blackImage);
		cvZero(disparity);
		cvZero(disparitySmaller);
		cvZero(disparityToShow);
		cvZero(trajectory);
		cvZero(trajectorySmaller);
	}
}

void MgrStereoHand::initFrames(){

	CvSize size = Settings::instance()->defSize;
	CvSize smaller = Settings::instance()->defSmallSize;

	for(int i = 0; i < 2; ++i){
		frameSmaller[i] = cvCreateImage(smaller, 8, 3);
		frameSmallerGray[i] = cvCreateImage(smaller, 8, 1);

		frameShow[i] = cvCreateImage(size, 8, 3);
		frameDiff[i] = cvCreateImage(size, 8, 1);
		frameBackground[i] = cvCreateImage(size, 8, 1);
		cvZero(frameBackground[i]);
		frameRectified[i] = cvCreateImage(size, 8, 3);

		frameGray[i] = cvCreateImage(size, 8, 1);
		frameSkin[i] = cvCreateImage(size, 8, 1);
		frameBlob[i] = cvCreateImage(size, 8, 1);
	}

	blackImage = cvCreateImage(smaller, 8, 1);
	cvZero(blackImage);

	disparity = cvCreateImage(size, IPL_DEPTH_8U, 1);
	disparitySmaller = cvCreateImage(smaller, 8, 1);
	disparityToShow = cvCreateImage(smaller, 8, 3);

	trajectory = cvCreateImage(size, 8, 1);
	trajectorySmaller = cvCreateImage(smaller, 8, 1);
	cvZero(trajectorySmaller);
}


void MgrStereoHand::initUI(){

	lastLoadDir = QDir("");

	// rozmiar przetwarzania
	sizeGroup = new QButtonGroup(this);
	sizeGroup->addButton(ui.radioButtonSize0, 0);
	sizeGroup->addButton(ui.radioButtonSize1, 1);
	Settings::instance()->initSize(sizeGroup->checkedId());

	processGroup = new QButtonGroup(this);
	processGroup->addButton(ui.processButtonRadio0, VIDEO);
	//processGroup->addButton(ui.processButtonRadio1, RECORD);
	processGroup->addButton(ui.processButtonRadio2, CAMERA);
	processType = processGroup->checkedId();

	
	connect(sizeGroup,	SIGNAL(buttonClicked (int)),
			this,		SLOT(sizeButtonClicked(int)));

	connect(processGroup,	SIGNAL(buttonClicked (int)),
			this,			SLOT(processTypeChanged(int)));

	// film
	connect(ui.buttonLoadFilm0,	SIGNAL(clicked()),
			this,				SLOT(loadFilmClicked0()));

	connect(ui.buttonLoadFilm1,	SIGNAL(clicked()),
			this,				SLOT(loadFilmClicked1()));

	// kalibracja
	connect(ui.buttonLoadCalibration,	SIGNAL(clicked()),
			this,						SLOT(loadCalibrationFile()));

	connect(ui.calibrateButton,			SIGNAL(clicked()),
			this,						SLOT(calibrateButtonClicked()));

	
	// start-stop
	connect(ui.buttonStart,		SIGNAL(clicked()),
			this,				SLOT(startProcess()));

	connect(ui.buttonStop,		SIGNAL(clicked()),
			this,				SLOT(stopProcess()));

	// pokazywane elementy przetwarzania
	connect(ui.sliderShowImage,	SIGNAL(valueChanged(int)),
			this,				SLOT(changeShowImage(int)));
	Settings::instance()->initImageType(ui.sliderShowImage->value());
	ui.labelShowImage->setText(Settings::instance()->getImageTypeString());

	// wyjscie
	connect(ui.buttonExit,		SIGNAL(clicked()),
			this,				SLOT(exit()));

	algorithmsDialog = new AlgorithmsDialog(this);
	statisticsDialog = new StatisticsDialog(this);
	calibDialog = new CalibrationDialog(this);

	connect(calibDialog,	SIGNAL(calibrationStartFromDialog()),
			this,			SLOT(startCalibrationClickedFromDialog()));

	connect(calibDialog,	SIGNAL(calibrationSaved()),
			this,			SLOT(savedCalibrationFromDialog()));


	setGeometry(10, 30, geometry().width(), geometry().height());
	
	algorithmsDialog->setGeometry(	10, geometry().height()+60, 
									algorithmsDialog->geometry().width(), 
									algorithmsDialog->geometry().height());

	statisticsDialog->setGeometry(	geometry().width()+20, 30, 
									statisticsDialog->geometry().width(), 
									statisticsDialog->geometry().height());

	calibDialog->setGeometry(	50, geometry().height(), 
									algorithmsDialog->geometry().width(), 
									algorithmsDialog->geometry().height());

	algorithmsDialog->show();
	statisticsDialog->show();
}


QString MgrStereoHand::loadFilm(int id){

	QString file = QFileDialog::getOpenFileName(NULL, tr("Wybierz film"), lastLoadDir.absolutePath(), tr("Filmy (*.avi)"));
	if(file != NULL){
		lastLoadDir = QDir(file);

		if(id == 0){
			ui.labelFilm0->setText(lastLoadDir.relativeFilePath(lastLoadDir.dirName()));
			fileFilm0 = file;
		}else{
			ui.labelFilm1->setText(lastLoadDir.relativeFilePath(lastLoadDir.dirName()));
			fileFilm1 = file;
		}

		ui.processButtonRadio0->click();
	}
	return file;
}


void MgrStereoHand::exit(){
	QApplication::exit(0);
}

// czy moze rozpoczac
bool MgrStereoHand::checkIfStart(){

	if(calibration)
		return true;

	if(processType != VIDEO){
		// czy jest kalibracja?
		if(fileCalib.isEmpty() && !calibModule->getCalibrationDone()){
			// trzeba najpierw przeprowadzic kalibracje
			QMessageBox::warning(NULL, "Kalibracja", "Aby rozpoczac przetwarzanie wczytaj plik z kalibracja lub skalibruj obraz");
			return false;
		}
		else{

			fileFilm0 = "";
			fileFilm1 = "";

			return true;
		}
	}
	else{
		if(fileFilm0.isEmpty() || fileFilm1.isEmpty()){
			QMessageBox::warning(NULL, "Filmy", "Aby rozpoczac przetwarzanie wczytaj pliki z filmami z obydwu kamer");
			return false;
		}

		if(fileCalib.isEmpty()){
			QMessageBox::warning(NULL, "Kalibracja", "Aby rozpoczac przetwarzanie wczytaj plik z kalibracja");
			return false;
		}
		return true;
	}
	return false;
}


void MgrStereoHand::loadFilmClicked0(){
	loadFilm(0);
	
}

void MgrStereoHand::loadFilmClicked1(){
	loadFilm(1);
}



void MgrStereoHand::loadCalibrationFile(){
	QString file = QFileDialog::getOpenFileName(NULL, tr("Wybierz dane kalibracji"),  Settings::instance()->lastLoadCalibDir.absolutePath(), tr("Kalibracja (*.calib)"));
	if(file != NULL){
		 Settings::instance()->lastLoadCalibDir = QDir(file);
		ui.labelCalibrationFile->setText( Settings::instance()->lastLoadCalibDir.relativeFilePath( Settings::instance()->lastLoadCalibDir.dirName()));
		fileCalib = file;
		QByteArray ba = file.toAscii();
		calibModule->calibrationLoad(ba.data());
	}
}

void MgrStereoHand::startCalibrationClickedFromDialog(){
	qDebug() << "start calib";
	calibration = true;
	nothing = false;
}


void MgrStereoHand::savedCalibrationFromDialog(){
	ui.labelCalibrationFile->setText( Settings::instance()->lastLoadCalibDir.relativeFilePath( Settings::instance()->lastLoadCalibDir.dirName()));
	fileCalib = Settings::instance()->lastLoadCalibDir.absolutePath();
}



void MgrStereoHand::startProcess(){
	nothing = false;
}


void MgrStereoHand::stopProcess(){
	nothing = true;
}
/*
void MgrStereoHand::startStopTrajectoryClicked(){
	if(!nothing){
		startRecognized[0]=(startRecognized[0] == 2 ? 3 : 2);
	}
}
*/

// zakladam, ze timer zostal zatrzymany
void MgrStereoHand::makeEverythingStop(){

	// nagranie
	if(processType == CAMERA && !calibration){
		recordFilms();
	}else if(!calibration){
		storeStatistics();
	}

	// wszystko co jest potrzebne do zastopowania
	setUIstartEnabled(false);
/*
	trajectoryWindow->hide();
	disparityWindow->hide();
*/
	nothing = true;

	if(calibration){
		calibration = false;
		ui.labelCalibrationFile->setText("plik nie zapisany");
		fileCalib = "";
	}

	// z powrotem kamerki
	if(processType == VIDEO){
		initCameras();
	}

	// i znowu zaczynamy petle idle
	framesTimer = new QTimer();
	connect(framesTimer, SIGNAL(timeout()), this, SLOT(mainIdleLoop()));
	framesTimer->start(1);
}

// zakladam, ze timer zostal zatrzymany
void MgrStereoHand::makeEverythingStart(){
	if(calibration){
		// USTAWIENIA KALIBRACJI
		calibModule->calibrationStart(	calibDialog->getBoardSize1(),
										calibDialog->getBoardSize2(),
										calibDialog->getSampleCounter());
	}
	reinitAll();
	setUIstartEnabled(true);

	// wracamy do glownej petli
	framesTimer = new QTimer();
	connect(framesTimer, SIGNAL(timeout()), this, SLOT(mainLoop()));
	framesTimer->start(1);
}


void MgrStereoHand::calibrateButtonClicked(){
	// przeprowadzenie kalibracji
	calibDialog->exec();
}


void MgrStereoHand::changeShowImage(int value){
	Settings::instance()->setImageType(value);
	ui.labelShowImage->setText(Settings::instance()->getImageTypeString());
}

void MgrStereoHand::sizeButtonClicked(int id){
	Settings::instance()->setSize(id);
}

void MgrStereoHand::processTypeChanged(int id){
	processType = id;
	ui.calibrateButton->setEnabled(id != VIDEO);
}

void MgrStereoHand::setUIstartEnabled(bool enable){
	ui.buttonStart->setEnabled(!enable);
	ui.groupBoxCalibration->setEnabled(!enable);
	ui.groupBoxSize->setEnabled(!enable);
	ui.groupBoxFilm->setEnabled(!enable);

	ui.buttonStop->setEnabled(enable);
}

void MgrStereoHand::recordFilms(){

	// 1. czy jest kalibracja
	//    - nie ma: trzeba zapisac, nie zapisac - KONIEC
	// 2. zapisanie 1 pliku
	//	  - nie zapisane - KONIEC
	// 3. zapisanie 2 pliku
	//	  - nie zapisane - KONIEC
	//
	// wyswietlenie info
	// videoWriter i zapisanie
	//
	// KONIEC - wyczyszczenie buforow

	leftCamWindow->showImage(blackImage);
	rightCamWindow->showImage(blackImage);

	displayOverlay(leftCamWindow->name, "Zapisywanie...", 100000);
	displayOverlay(rightCamWindow->name, "Zapisywanie...", 100000);

	QString file1 = QFileDialog::getSaveFileName(NULL, tr("Zapisz film 1"), lastLoadDir.absolutePath(), tr("Filmy (*.avi)"));
	QString file2;
	
	// STATS
	statisticsDialog->fps = fps;
	statisticsDialog->bps = bps;
	statisticsDialog->allBits = allBits;
	statisticsDialog->allFrames = numFrames;
	statisticsDialog->file1 = "";
	statisticsDialog->file2 = "";
	statisticsDialog->calibration = fileCalib;

	bool saved = false;
	if(file1 != NULL && !file1.isEmpty()){
		file2 = QFileDialog::getSaveFileName(NULL, tr("Zapisz film 2"), file1, tr("Filmy (*.avi)"));
		if(file2 != NULL && !file2.isEmpty()){
			
			if(fileCalib.isEmpty()){
				// proba zapisania
				file1.chop(4);
				QByteArray ba = file1.append(".calib").toAscii();
				calibModule->calibrationSave(ba.data());
				QMessageBox::information(NULL, "Kalibracja", QString("Plik kalibracji zapisany do pliku ").append(file1));
				fileCalib = file1;
			}
			// STATS
			statisticsDialog->calibration = fileCalib;
			statisticsDialog->file1 = file1;
			statisticsDialog->file2 = file2;
			

			QByteArray ba1 = file1.toAscii();
			QByteArray ba2 = file2.toAscii();
			CvVideoWriter * videoWriter0 = cvCreateAVIWriter(ba1.data(), CV_FOURCC('I', 'Y', 'U', 'V'), 15, cvSize(640,480));
			CvVideoWriter * videoWriter1 = cvCreateAVIWriter(ba2.data(), CV_FOURCC('I', 'Y', 'U', 'V'), 15, cvSize(640,480));
	
			for(int i = 0; i < buffer[0].size() && i < buffer[1].size(); ++i){
				cvWriteFrame( videoWriter0, buffer[0].at(i) );
				cvWriteFrame( videoWriter1, buffer[1].at(i) );
			}

			cvReleaseVideoWriter(&videoWriter0);
			cvReleaseVideoWriter(&videoWriter1);

			statisticsDialog->trajectory = saveTrajectory();

			saved = true;
		}
	}
	
	statisticsDialog->showStatistics();

	displayOverlay(leftCamWindow->name, saved ? "Zapisano filmy" : "Operacja anulowana", 2000);
	displayOverlay(rightCamWindow->name, saved ? "Zapisano filmy" : "Operacja anulowana", 2000);

	for(int i = 0; i < buffer[0].size() && i < buffer[1].size(); ++i){
		cvReleaseImage(&buffer[0][i] );
		cvReleaseImage(&buffer[1][i] );
	}

	buffer[0].clear();
	buffer[1].clear();
}


void MgrStereoHand::storeStatistics(){
	leftCamWindow->showImage(blackImage);
	rightCamWindow->showImage(blackImage);

	displayOverlay(leftCamWindow->name, "Zapisywanie...", 100000);
	displayOverlay(rightCamWindow->name, "Zapisywanie...", 100000);
	
	// STATS
	statisticsDialog->fps = fps;
	statisticsDialog->bps = bps;
	statisticsDialog->allBits = allBits;
	statisticsDialog->allFrames = numFrames;
	statisticsDialog->file1 = fileFilm0;
	statisticsDialog->file2 = fileFilm1;
	statisticsDialog->calibration = fileCalib;
	statisticsDialog->trajectory = saveTrajectory();

	statisticsDialog->showStatistics();

	displayOverlay(leftCamWindow->name, "Zapisano" , 1000);
	displayOverlay(rightCamWindow->name, "Zapisano", 1000);

	buffer[0].clear();
	buffer[1].clear();
}


QString MgrStereoHand::saveTrajectory(){
	
	QString file = QFileDialog::getSaveFileName(NULL, tr("Zapisz trajektorie"), lastLoadDir.absolutePath(), tr("Plik trajektorii (*.trj)"));
	if(file == NULL){
		file = "trajektoria.trj";
	}
	
	QFile fileTrj(file);

	if (fileTrj.open(QIODevice::WriteOnly)){

		QTextStream in(&fileTrj);

		// wysokosc i szerokosc obrazu
		in << Settings::instance()->defSize.width;
		in << "\n";
		in << Settings::instance()->defSize.height;
		in << "\n";

		// ilosc punktow
		in << hand[0]->allXYZ.size();
		in << "\n";

		//for(int k = 0; k < 1; ++k){
		for(int i = 0; i < hand[0]->allXYZ.size(); ++i){
			
			if(	hand[0]->allXYZ[i].x >= -1 && 
				hand[0]->allXYZ[i].y >= -1 &&
				hand[0]->allXYZ[i].z >= -1){

				in << hand[0]->allXYZ[i].x <<",";
				in << hand[0]->allXYZ[i].y <<",";
				in << hand[0]->allXYZ[i].z <<"\n";
			}
			//qDebug() << hand[0]->allXYZ[i].x;
			//qDebug() << hand[0]->allXYZ[i].y;
			//qDebug() << hand[0]->allXYZ[i].z;
		}

	/*
		QDataStream in(&fileTrj);

		// wysokosc i szerokosc obrazu
		in << (qint32)Settings::instance()->defSize.width;
		in << (qint32)Settings::instance()->defSize.height;

		// ilosc punktow
		in << (qint32)hand[0]->allXYZ.size();

		//for(int k = 0; k < 1; ++k){
		for(int i = 0; i < hand[0]->allXYZ.size(); ++i){
			in << (qint32)hand[0]->allXYZ[i].x;
			in << (qint32)hand[0]->allXYZ[i].y;
			in << (qint32)hand[0]->allXYZ[i].z;
		}
		//}
		*/

		fileTrj.close();
		return file;
	}
	return "";

}


void MgrStereoHand::updateFPS(){
	numFrames++;
	allBits += (frame[0]->imageSize);
	//float currentUpdate = getGameTime();
	if(startTime == 0)
		time(&startTime);

	time(&endTime);
	fps = numFrames /(difftime(endTime, startTime));
	bps = allBits / (difftime(endTime, startTime));
	/*
	if( currentUpdate - lastUpdate > fpsUpdateInterval ) {
		fps = numFrames / (currentUpdate - lastUpdate);
		bps = allBits / (currentUpdate - lastUpdate);
		lastUpdate = currentUpdate;
		numFrames = 0;
	}*/
}

void MgrStereoHand::initGameTime(){

	startTime = 0;

	avgFindSkinTime = 0;
	avgRectifyTime = 0;
	avgFindHandTime = 0;
	avgStereoTime = 0;

	beginActionTime = 0;
	endActionTime = 0;
/*
	if( !QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSecond) )
		ticksPerSecond = 1000;
	timeAtGameStart = 0;
	timeAtGameStart = getGameTime();
*/
}

// Called every time you need the current game time
float MgrStereoHand::getGameTime(){
	UINT64 ticks;
	float time;
	// This is the number of clock ticks since start
	if( !QueryPerformanceCounter((LARGE_INTEGER *)&ticks) )
	ticks = GetTickCount();//(UINT64)timeGetTime();
	// Divide by frequency to get the time in seconds
	time = (float)(__int64)ticks/(float)(__int64)ticksPerSecond;
	// Subtract the time at game start to get
	// the time since the game started
	time -= timeAtGameStart;
	return time;
}

/*
		calibModule->rectifyImages(frame, frameRectified);

		if(processType == CAMERA){
			buffer[0].push_back(frameRectified[0]);
			buffer[1].push_back(frameRectified[1]);
		}

		cvCopyImage(frameRectified[0], frameShow[0]);
		cvCopyImage(frameRectified[1], frameShow[1]);

		if(stateHist == STATE_AFTER_HIST && stateBack == STATE_AFTER_BACK){
			
			// tutaj - juz spokojnie tracking dloni i cale przetwarzanie
			// dlatego ma nawet osobna petle
			bool handFound = true;
			for(int i = 0; i < 2; ++i){
				
				IplImage * frameToUse = frameRectified[i];

				// usuwanie tla
				cvCvtColor(frameToUse, frameGray[i], CV_BGR2GRAY);
				//if(Settings::getInstance().framesCounter % 200 == 0)
					//cvAddWeighted(frameGray[i], ALPHA_BG, frameBackground[i], (1.0-ALPHA_BG), 0.0, frameBackground[i]);
				cvAbsDiff(frameGray[i], frameBackground[i], frameDiff[i]);
				cvThreshold(frameDiff[i], frameDiff[i], backgroundThreshold, 255, CV_THRESH_BINARY);

				// potencjalny prostokat ograniczajacy dlon
				// albo wiekszy od poprzedniego albo na lewo od twarzy
				CvRect rect =	//(hand[i]->lastRect.height == -1) ? 
								cvRect(	0, 
										faceDetection[i]->lastFound.y, 
										faceDetection[i]->lastFound.x,
										Settings::instance()->defSize.height - faceDetection[i]->lastFound.y);// : 
								//hand[i]->getBiggerRect();

				// teraz wykrywanie skory
				skinDetection[i]->detectSkin(frameToUse, frameSkin[i], rect, segmantationAlg, frameDiff[i]);

				// i wykrywanie dloni
				handFound = handFound && handDetection[i]->findHand(frameSkin[i], frameBlob[i], rect, *hand[i]);
			}

			// a tutaj juz skladamy obraz stereo, jesli sa znalezione dlonie
			handFound = false;
			if(handFound){
				if(stereoAlg < MINE_)
					stereoModule->stereoProcessGray(frameGray, frameBlob, hand, disparity, stereoAlg);
				else
					stereoModule->stereoProcessMine(frameGray, frameBlob, hand, disparity, stereoAlg);
				
				cvResize(disparity, disparitySmaller, CV_INTER_NN);
				drawingModule->drawDispOnFrame(hand[0]->lastZ, disparitySmaller, disparityToShow);
				disparityWindow->showImage(disparityToShow);
			}else{
				hand[0]->setLastPointWithZ(-1);
				hand[1]->setLastPointWithZ(-1);

				cvZero(disparityToShow);
				disparityWindow->showImage(disparityToShow);
			}

			drawingModule->drawTrajectoryOnFrame(hand[0], trajectorySmaller);
			trajectoryWindow->showImage(trajectorySmaller);
			
		}
		else if(stateHist != STATE_BEFORE_HIST && stateHist != STATE_AFTER_HIST){
			
			for(int i = 0; i < 2; ++i){
				// pobieranie histogramu skory
				// pierwsze sprawdzamy to, czy juz jest to zrobione, aby ograniczyc wywolanie porownan
				
				if(faceDetection[i]->findHeadHaar(frameRectified[i])){
					// jak znaleziono - update histogramu
					skinDetection[i]->updateHistogram(frameRectified[i], faceDetection[i]->head.getSmallerRect());
					// no i narysujmy ja
					cvRectangleR(frameShow[i], faceDetection[i]->head.lastRect, cvScalar(0, 0, 255), 2);
				}
				drawingModule->drawSamplesOnFrame(skinDetection[i]->getSampleCount(), frameShow[i]);

				// jak juz jest wystarczajaco duzo probek - koniec
				if(	skinDetection[i]->getSampleCount() >= 5){
					skinDetection[i]->skinFound = true;

					// ustawienie wskaznika na twarz, aby mozna bylo znalezc dlon
					handDetection[i]->head = &faceDetection[i]->head;
				}
			}
			// po przejsciu calej petli - jesli mamy juz w obydwu znaleziona skore -konczymy
			if(skinDetection[0]->skinFound && skinDetection[1]->skinFound){
				stateHist = STATE_AFTER_HIST;
				stateBack = STATE_BACK;
				currentBgFrames = 0;
			}
		}
		else if(stateBack == STATE_BACK){
			for(int i = 0; i < 2; ++i){
				
				cvCvtColor(frameRectified[i], frameGray[i], CV_BGR2GRAY);
				cvAddWeighted(frameGray[i], ALPHA_BG, frameBackground[i], (1.0-ALPHA_BG), 0.0, frameBackground[i]);
				
			}
			currentBgFrames++;

			if(currentBgFrames >= BG_UPDATE_FRAMES){
				stateBack = STATE_AFTER_BACK;
			}
		}
*/