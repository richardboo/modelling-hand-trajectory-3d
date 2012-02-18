#include "processingthread.h"
#include "settings.h"
#include "imageutils.hpp"

#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>

using namespace std;
using namespace cv;

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

static int segmantationAlg;
static int stereoAlg;

float ALPHA_BG = 0.07f;
int BG_UPDATE_FRAMES = 10;
int BKG_THRESH = 30;


ProcessingThread::ProcessingThread(QObject *parent)
	: QObject(parent)
{
	

	currentBgFrames = 0;

	srModule = new SignRecognitionModule("A.xml");
	startRecognized[0] = startRecognized[1] = 0;
	lastStart[0] = lastStart[1] = false;

	hand[0] = hand[1] = head[0] = head[0] = NULL;
	frameGrabber[0] = frameGrabber[1] = NULL;

	stop = false;
	calibration = false;
	nothing = true;
	prevNothing = true;
	segmantationAlg = 0;
	stereoAlg = 0;

	lastUpdate        = 0;
	fpsUpdateInterval = 0.5f;
	numFrames         = 0;
	fps               = 0;
	startTime		  = 0;
	endTime			  = 0;
	ticksPerSecond	  = 0;
	timeAtGameStart   = 0;
	counterPoints		= 0;

	startProcessTime = 0;
	endProcessTime = 0;

	skinTime = 0;
	counterSkin = 0;
	stereoTime = 0;

	framesStartStopCounter = 0;
	framesProcessingCounter = 0;

	duringRecognition = false;

	drawingModule = new DrawingModule;
}

ProcessingThread::~ProcessingThread(){
	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		// ONE
		delete frameGrabber[1];
	}

	for(int i = 0; i < 2; ++i){
		if(hand[i] != NULL)
			delete hand[i];
		if(head[i] != NULL)
			delete head[i];
	}
	delete drawingModule;
	delete srModule;
	
}

void ProcessingThread::init(FrameStorage * f, CalibrationModule * calibMod, CalibrationDialog * dial,
				StatisticsDialog * statDial){

	Settings::instance()->defSize = cvSize(640, 480);
	fs = f;
	calibModule = calibMod;
	calibDialog = dial;
	statisticsDialog = statDial;

	StereoModule::initStates();

	initFrames();
	initCameras();
	initModules();

	rect = cvRect(0, 0, 
						Settings::instance()->defSize.width,
						Settings::instance()->defSize.height);
}

void ProcessingThread::process(){

	while(true){
	
        // Stop thread if stopped=TRUE //
        /////////////////////////////////
        stopMutex.lock();
        if (stop){
            stop=false;
            stopMutex.unlock();
			qDebug() << "EXIT";
            break;
        }
        stopMutex.unlock();
		bool isok = true;
        /////////////////////////////////
		try{
		if(prevNothing){
			isok = mainIdleLoop();
			// moze juz koniec idle?
			bool not2 = getNothing();
			if(!not2){
				if(!checkIfStart()){
					//qDebug() << "No trudno";
					setNothing(true);
					continue;
				}
				prevNothing = not2;
				makeEverythingStart();
				
			}
		}
		else{
			mainLoop();
			// moze koniec przetwarzania?
			bool not2 = getNothing();
			prevNothing = not2;
			if(not2){
				// zatrzymujemy
				time(&endTime);
				makeEverythingStop();
			}
			if(processType == VIDEO)
				Sleep(50);
		}
		}catch(Exception ex){
			
		}
		if(!isok)
			Sleep(50);
	}
	emit finished();
}

bool ProcessingThread::mainLoop(){

	for(int i = 0; i < 2; ++i){
		if(!frameGrabber[i]->hasNextFrame()){
			// jesli jest video to zatrzymanie
			// i zebranie statystyk
			if(processType == VIDEO){
				nothing = true;
			}
			
			return false;
		}
		frame[i] = frameGrabber[i]->getNextFrame();
	}
	
	framesCounter++;
	updateFPS();
	
	if(!getCalibration()){
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
		#pragma omp parallel for shared(frameRectified,frameShow,frame,frameSkin,segmantationAlg,frameDiff) private(i)
		for(i = 0; i < 2; ++i){

			//qDebug() << "Hello from thread " << omp_get_thread_num();
			calibModule->rectifyImage(frame[i], frameRectified[i], i);
			cvCopyImage(frame[i], frameShow[i]);
			cvCvtColor(frameRectified[i], frameGray[i], CV_BGR2GRAY);
		}

//////////////// STAT ///////////////////////////
		/*
		counterTillHandGet--;
		if(counterTillHandGet == 0){
			cvCopyImage(frame[0], imageHand[0]);
			cvCopyImage(frame[1], imageHand[1]);
			emit showOverlay("Pobrana probka dloni, trwa inicjalizacja", 3000);
		}
		else if(counterTillHandGet > 0){
			return true;
		}*/
/////////////////////////////////////////////////

		if((segmantationAlg == HIST_ || segmantationAlg == ALL_) && stateHist != STATE_AFTER_HIST){
			
			for(int i = 0; i < 2; ++i){
				
				// pobieranie histogramu skory
				// pierwsze sprawdzamy to, czy juz jest to zrobione, aby ograniczyc wywolanie porownan
				
				if(faceDetection[i]->findHeadHaar(frameRectified[i], head[i])){
					// jak znaleziono - update histogramu
					skinDetection[i]->updateHistogram(frameRectified[i], head[i]->getSmallerRect());
					// no i narysujmy ja
					cvRectangleR(frameShow[i], head[i]->lastRect, cvScalar(0, 0, 255), 2);
				}
				drawingModule->drawSamplesOnFrame(skinDetection[i]->getSampleCount(), frameShow[i]);

				// jak juz jest wystarczajaco duzo probek - koniec
				if(	skinDetection[i]->getSampleCount() >= 10){
					skinDetection[i]->skinFound = true;

					// ustawienie wskaznika na twarz, aby mozna bylo znalezc dlon
					//head[i] = &faceDetection[i]->head;
				}

				// po przejsciu calej petli - jesli mamy juz w obydwu znaleziona skore -konczymy
				if(skinDetection[0]->skinFound && skinDetection[1]->skinFound){
					stateHist = STATE_AFTER_HIST;
				}
			}
		}
		else if(segmantationAlg == ONLY_BG_ /*|| segmantationAlg == BGSKIN_*/){
			
			if(stateBack == STATE_BEFORE_BACK){
				for(int i = 0; i < 2; ++i){
					
					cvAddWeighted(frameGray[i], ALPHA_BG, frameBackground[i], (1.0-ALPHA_BG), 0.0, frameBackground[i]);
					if(faceDetection[i]->lastFound.x == -1){
						faceDetection[i]->findHeadHaar(frameRectified[i], head[i]);
					}
				}
				currentBgFrames++;

				// wylaczamy dopiero jak chociaz raz byla rozpoznana twarz
				if(currentBgFrames >= BG_UPDATE_FRAMES  
					/*faceDetection[0]->lastFound.x > -1 &&
					faceDetection[1]->lastFound.x > -1*/){

					stateBack = STATE_AFTER_BACK;

					if(faceDetection[0]->lastFound.x != -1 && faceDetection[1]->lastFound.x != -1){

						cvSetImageROI(frameBackground[0], faceDetection[0]->lastFound);
						cvSetImageROI(frameBackground[1], faceDetection[1]->lastFound);
						cvSet(frameBackground[0], cvScalarAll(0));
						cvSet(frameBackground[1], cvScalarAll(0));
						cvResetImageROI(frameBackground[0]);
						cvResetImageROI(frameBackground[1]);
					}
				}
			}
			else{

				#pragma omp parallel for default (shared) private(i)
				for(int i = 0; i < 2; ++i){
					cvCvtColor(frameRectified[i], frameGray[i], CV_BGR2GRAY);
					if(framesCounter % 50 == 0)
						cvAddWeighted(frameGray[i], ALPHA_BG, frameBackground[i], (1.0-ALPHA_BG), 0.0, frameBackground[i]);
					cvAbsDiff(frameGray[i], frameBackground[i], frameDiff[i]);
					cvThreshold(frameDiff[i], frameDiff[i], BKG_THRESH, 255, CV_THRESH_BINARY);
				}
			}
		}

		int handNotFound[2];
		handNotFound[0] = 3;
		handNotFound[1] = 3;
		bool changeStart = false;
		
//////////////// STAT ///////////////////////////
		time(&tempStart);
/////////////////////////////////////////////////
		#pragma omp parallel for shared(changeStart, frameRectified,frameShow,frame,frameBlob,frameSkin,segmantationAlg,frameDiff) private(i)
		for(int i = 0; i < 2; ++i){

			skinDetection[i]->detectSkin(frameRectified[i], frameSkin[i], rect, segmantationAlg, frameDiff[i]);
			handNotFound[i] = handDetection[i]->findHand(frameSkin[i], frameBlob[i], frame[i], rect, *hand[i], *head[i]);
			
			if(!handNotFound[i]){
				bool start = srModule->isSign(frameBlob[i], hand[i]->lastRect);

				if(start && startRecognized[i] != 1){
					startRecognized[i]++;
				}
				else if(!start && startRecognized[i] == 1){
					// raz nie rozpoznano, wiec mozna przejsc do stanu ROZPOZNAWANIE
					startRecognized[i]++;
				}
			}
		}
//////////////// STAT ///////////////////////////
		time(&tempEnd);
		skinTime += difftime(tempEnd, tempStart);
		counterSkin++;
/////////////////////////////////////////////////


		// stan obydwu to minimum z nich
		int minRec = min(startRecognized[0], startRecognized[1]);
		startRecognized[0] = startRecognized[1] = minRec;

		if(Settings::instance()->changeTrajectory){
			startRecognized[0] = (startRecognized[0] == 0 ? 2 : startRecognized[0]+1);
			startRecognized[1] = startRecognized[0];
			Settings::instance()->changeTrajectory = false;
		}

		//qDebug() << "i: " << startRecognized[0] << " " << startRecognized[1];

//////////////// STAT ///////////////////////////
		if(duringRecognition){
			framesStartStopCounter++;
		}
/////////////////////////////////////////////////
		//stereoModule->stereoProcessGray(frameGray, frameBlob, hand, disparity, stereoAlg);
		//cvResize(disparity, disparitySmaller);

		if(!handNotFound[0] && !handNotFound[1] && (startRecognized[0] == 2)){

//////////////// STAT ///////////////////////////
			if(!duringRecognition){
				emit showOverlay( "START RECOGNIZED", 1000);			
				duringRecognition = true;
				time(&startProcessTime);
			}
			
			time(&tempStart);
/////////////////////////////////////////////////

			if(stereoAlg < MINE_)
				stereoModule->stereoProcessGray(frameGray, frameBlob, hand, disparity, stereoAlg);
			else
				stereoModule->stereoProcessMine(frameGray, frameBlob, hand, disparity, stereoAlg);
	
//////////////// STAT ///////////////////////////
			framesProcessingCounter++;
			time(&tempEnd);
			stereoTime += difftime(tempEnd, tempStart);
/////////////////////////////////////////////////

			calibModule->setRealCoordinates(hand);
			cvResize(disparity, disparitySmaller);
			//STEREO
			drawingModule->drawDispOnFrame(hand[0]->lastDisp, disparitySmaller, disparityToShow);
			if(stereoAlg == MINE_RND_){
				drawingModule->drawPointsOnDisp(disparityToShow, stereoModule->mpts_2, hand[1]);
				counterPoints += stereoModule->mpts_1.size();
			}
			

		}else{
			hand[0]->setLastPointWithZ(-1);
			hand[1]->setLastPointWithZ(-1);

			cvZero(disparityToShow);

			if(startRecognized[0] >= 3){
				emit showOverlay( "STOP RECOGNIZED", 1000);
				
//////////////// STAT ///////////////////////////
				duringRecognition = false;
				time(&endProcessTime);
/////////////////////////////////////////////////
			}
		}

		// odrysowanie trajektorii
		drawingModule->drawTrajectoryOnFrame(hand[0], trajectorySmaller);


//////////////// STAT ///////////////////////////
		if(framesCounter % 10 == 0){

			for(int i = 0; i < 2; ++i){
				IplImage * copy = cvCloneImage(frameSkin[i]);
				bufferSkin[i].push_back(copy);

				IplImage * copy2 = cvCreateImage(Settings::instance()->defSize, 8, 3);
				cvZero(copy2);
				cvCopy(frameRectified[i], copy2, frameBlob[i]);
				bufferHand[i].push_back(copy2);

				IplImage * copy3 = cvCloneImage(frameRectified[i]);
				bufferRectified[i].push_back(copy3);
			}
			IplImage * copy4 = cvCloneImage(disparityToShow);
			bufferDepth.push_back(copy4);
		}
/////////////////////////////////////////////////

	}
	else{
	// KALIBRACJA

		// kalibracja, bierzemy co 20 klatke
		if(framesCounter % 10 == 0){

			// migniecie
			cvZero(frameShow[0]);
			cvZero(frameShow[1]);
			emit showImages();

			// kalibracja
			ImageUtils::getGray(frame[0], frameGray[0]);
			ImageUtils::getGray(frame[1], frameGray[1]);

			calibModule->calibrationAddSample(frameGray[0], frameGray[1]);
			
			if(calibModule->getSampleCount() == calibModule->getMaxSamples()){

				//framesTimer->stop();
				emit showOverlay("Kalibracja...", 2000);
				calibModule->calibrationEnd();
				//calibDialog->endCalibration();
				//setCalibration(false);
				setNothing(true);
			}
			else{
				return true;
			}
		}
		cvCopyImage(frame[0], frameShow[0]);
		cvCopyImage(frame[1], frameShow[1]);
	}

	//cvCopyImage(frame[0], frameShow[0]);
	//cvCopyImage(frame[1], frameShow[1]);

	emit showImages();
	return true;
}

bool ProcessingThread::mainIdleLoop(){
	bool ret = true;
	if(frameGrabber[0] != NULL && frameGrabber[1] != NULL){

		for(int i = 0; i < 2; ++i){
			if(!frameGrabber[i]->hasNextFrame()){
				return false;
			}
			frame[i] = frameGrabber[i]->getNextFrame();
		}

		cvCopyImage(frame[0], frameShow[0]);
		cvCopyImage(frame[1], frameShow[1]);
		cvZero(trajectorySmaller);
		cvZero(disparitySmaller);
	}
	else{
		cvZero(frameShow[0]);
		cvZero(frameShow[1]);
		cvZero(trajectorySmaller);
		cvZero(disparitySmaller);
		ret = false;
	}
	framesCounter++;
	updateFPS();
	emit showImages();
	return ret;
}


bool ProcessingThread::initCameras(){
	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		// ONE
		delete frameGrabber[1];
	}

	CameraDevice * cam1 = new CameraDevice(0);
	//CameraDevice * cam2 = cam1;
	// ONE
	CameraDevice * cam2 = new CameraDevice(1);

	// ONE
	cam1->init(Settings::instance()->defSize.width,
				   Settings::instance()->defSize.height);


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

	fs->frame[0] = cam1->getNextFrame();
	fs->frame[1] = cam2->getNextFrame();

	return true;
}

void ProcessingThread::initModules(){
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
}

bool ProcessingThread::initFilms(){
	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		//ONE
		delete frameGrabber[1];
	}

	//qDebug() << Settings::instance()->fileFilm0 << Settings::instance()->fileFilm1;

	VideoGrabber * film1 = new VideoGrabber(Settings::instance()->fileFilm0);
	VideoGrabber * film2 = new VideoGrabber(Settings::instance()->fileFilm1);

	if(!film1->init() || !film2->init())
		return false;

	frameGrabber[0] = film1;
	frameGrabber[1] = film2;

	film1->hasNextFrame();
	film2->hasNextFrame();

	fs->frame[0] = film1->getNextFrame();
	fs->frame[1] = film2->getNextFrame();

	return true;
}

void ProcessingThread::initFrames(){
	for(int i = 0; i < 2; ++i){
		frame[i] = fs->frame[i];
		frameSmaller[i] = fs->frameSmaller[i];
		frameShow[i] = fs->frameShow[i];
		frameDiff[i] = fs->frameDiff[i];
		frameRectified[i] = fs->frameRectified[i];
		frameBackground[i] = fs->frameBackground[i];
		frameGray[i] = fs->frameGray[i];
		frameSmallerGray[i] = fs->frameSmallerGray[i];
		frameSkin[i] = fs->frameSkin[i];
		frameBlob[i] = fs->frameBlob[i];
	}
	blackImage = fs->blackImage;
	disparity = fs->disparity;
	disparitySmaller = fs->disparitySmaller;
	disparityToShow = fs->disparityToShow;
	trajectory = fs->trajectory;
	trajectorySmaller = fs->trajectorySmaller;
}

void ProcessingThread::setExit(){
    // Stop thread //
    /////////////////////////////////
    stopMutex.lock();
	stop = true;
    stopMutex.unlock();
    /////////////////////////////////
}

// czy moze rozpoczac
bool ProcessingThread::checkIfStart(){

	if(calibration)
		return true;

	if(processType != VIDEO){
		// czy jest kalibracja?
		if(Settings::instance()->fileCalib.isEmpty() && !calibModule->getCalibrationDone()){
			// trzeba najpierw przeprowadzic kalibracje
			emit messageError("Kalibracja", "Aby rozpoczac przetwarzanie wczytaj plik z kalibracja lub skalibruj obraz");
			//QMessageBox::warning(NULL, "Kalibracja", "Aby rozpoczac przetwarzanie wczytaj plik z kalibracja lub skalibruj obraz");
			return false;
		}
		else{
			return true;
		}
	}
	else{
		if(Settings::instance()->fileFilm0.isEmpty() || Settings::instance()->fileFilm1.isEmpty()){
			emit messageError("Filmy", "Aby rozpoczac przetwarzanie wczytaj pliki z filmami z obydwu kamer");
			//QMessageBox::warning(NULL, "Filmy", "Aby rozpoczac przetwarzanie wczytaj pliki z filmami z obydwu kamer");
			return false;
		}

		if(Settings::instance()->fileCalib.isEmpty()){
			QMessageBox::warning(NULL, "Kalibracja", "Aby rozpoczac przetwarzanie wczytaj plik z kalibracja");
			return false;
		}
		return true;
	}
	return false;
}

// zakladam, ze timer zostal zatrzymany
void ProcessingThread::makeEverythingStop(){

	if(endProcessTime == 0){
		time(&endProcessTime);
	}

	bool cal = getCalibration();
	// nagranie
	if(processType == CAMERA && !cal){
		//recordFilms();
		emit getFilmFileName();
	}else if(!cal){
		storeStatistics();
	}
	setNothing(true);

	if(cal){
		setCalibration(false);
		// plik nie zapisany
		emit calibrationNotSet();
		Settings::instance()->fileCalib = "";
	}

	if(processType == VIDEO){
		initCameras();
	}

	// reinicjalizacja fps
	framesCounter = 0;
	fps = 0;
	numFrames = 0;
	initGameTime();

	emit finishedProcess();
	// wszystko co jest potrzebne do zastopowania
}


void ProcessingThread::makeEverythingStart(){
	if(getCalibration()){
		// USTAWIENIA KALIBRACJI
		calibModule->calibrationStart(	calibDialog->getBoardSize1(),
										calibDialog->getBoardSize2(),
										calibDialog->getSampleCounter(),
										calibDialog->getRealSize());
	}
	reinitAll();
	emit startedProcess();
}

void ProcessingThread::recordFilms(QString file1){

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
	cvZero(frameShow[0]);
	cvZero(frameShow[1]);
	emit showImages();
	emit showOverlay("Zapisywanie...", 100000);

	// UWAGA!
	// ostatni dir
	//QString file1;// = QFileDialog::getSaveFileName(NULL, tr("Zapisz film"), Settings::instance()->lastLoadDir.absolutePath(), tr("Filmy (*.avi)"));
	QString file2;

	bool saved = false;
	if(file1 != NULL && !file1.isEmpty()){
		file1.chop(4);
		file2 = QString(file1);
		file1.append("_0.avi");
		file2.append("_1.avi");

		Settings::instance()->fileFilm0 = file1;
		Settings::instance()->fileFilm1 = file2;

		if(file2 != NULL && !file2.isEmpty()){
			
			if(Settings::instance()->fileCalib.isEmpty()){
				// proba zapisania
				file1.chop(4);
				QByteArray ba = file1.append(".calib").toAscii();
				calibModule->calibrationSave(ba.data());
				QMessageBox::information(NULL, "Kalibracja", QString("Plik kalibracji zapisany do pliku ").append(file1));
				Settings::instance()->fileCalib = file1;
			}

			QByteArray ba1 = file1.toAscii();
			QByteArray ba2 = file2.toAscii();
			CvVideoWriter * videoWriter0 = cvCreateAVIWriter(ba1.data(), CV_FOURCC('I', 'Y', 'U', 'V'), 15, cvSize(640,480));
			CvVideoWriter * videoWriter1 = cvCreateAVIWriter(ba2.data(), CV_FOURCC('I', 'Y', 'U', 'V'), 15, cvSize(640,480));
	
			for(int i = 0; i < buffer[0].size() && i < buffer[1].size(); ++i){
				cvWriteFrame( videoWriter0, buffer[0].at(i) );
				cvWriteFrame( videoWriter0, buffer[0].at(i) );
				cvWriteFrame( videoWriter1, buffer[1].at(i) );
				cvWriteFrame( videoWriter1, buffer[1].at(i) );
			}

			cvReleaseVideoWriter(&videoWriter0);
			cvReleaseVideoWriter(&videoWriter1);

			saved = true;
		}
	}
	else{
		Settings::instance()->fileFilm0 = "";
		Settings::instance()->fileFilm1 = "";
	}

	// zebranie statystyk
	storeStatistics();

	QString file = statisticsDialog->getFileName();
	statisticsDialog->trajectory = saveTrajectory(file);
	statisticsDialog->showStatistics();
	saveStatistics(file);

	emit showOverlay(saved ? "Zapisano filmy" : "Operacja anulowana", 2000);

	for(int i = 0; i < buffer[0].size() && i < buffer[1].size(); ++i){
		cvReleaseImage(&buffer[0][i] );
		cvReleaseImage(&buffer[1][i] );
	}

	buffer[0].clear();
	buffer[1].clear();
}


void ProcessingThread::storeStatistics(){
	cvZero(frameShow[0]);
	cvZero(frameShow[1]);
	emit showImages();
	emit showOverlay("Zapisywanie...", 100000);

	QString file = statisticsDialog->getFileName();
	QDir fileDir = QDir(file);
	fileDir.cd("stats");
	fileDir.mkdir(file);
	fileDir.cd(file);

	for(int i = 0; i < 2; ++i){
		for(int k = 0; k < bufferSkin[i].size(); ++k){
		
			QString fileSkin = fileDir.absolutePath()+"/skin_f"+QString().number(k)+"c"+QString().number(i)+".jpg";
			cvSaveImage(fileSkin.toStdString().c_str(), bufferSkin[i][k]);

			QString fileRect = fileDir.absolutePath()+"/rect_f"+QString().number(k)+"c"+QString().number(i)+".jpg";
			cvSaveImage(fileRect.toStdString().c_str(), bufferRectified[i][k]);

			QString fileHand = fileDir.absolutePath()+"/hand_f"+QString().number(k)+"c"+QString().number(i)+".jpg";
			cvSaveImage(fileHand.toStdString().c_str(), bufferHand[i][k]);
		}
	}

	for(int i = 0; i < bufferDepth.size(); ++i){
			QString fileDepth = fileDir.absolutePath()+"/disp_f"+QString().number(i)+".jpg";
			cvSaveImage(fileDepth.toStdString().c_str(), bufferDepth[i]);
	}

	/*
	cvSaveImage(QString(file+("_screen0.jpg")).toStdString().c_str(), imageHand[0]);
	cvSaveImage(QString(file+("_screen1.jpg")).toStdString().c_str(), imageHand[1]);
	*/

	statisticsDialog->fps = fps;
	statisticsDialog->file1 = Settings::instance()->fileFilm0;
	statisticsDialog->file2 = Settings::instance()->fileFilm1;
	statisticsDialog->calibration = Settings::instance()->fileCalib;
	statisticsDialog->stats = fileDir.absolutePath();

	statisticsDialog->timeProcess = (int)difftime(endProcessTime, startProcessTime);
	statisticsDialog->framesProcess = framesStartStopCounter;
	statisticsDialog->framesProper = framesProcessingCounter > framesStartStopCounter ? framesStartStopCounter : framesProcessingCounter;

	statisticsDialog->timeSkin = skinTime*1000.0f/(float)counterSkin;
	statisticsDialog->timeStereo = stereoTime*1000.0f/(float)framesProcessingCounter;

	statisticsDialog->trajectory = saveTrajectory(fileDir.absolutePath()+"/");

	statisticsDialog->counterPoints = (float)counterPoints/(float)framesProcessingCounter;
	
	statisticsDialog->showStatistics();
	saveStatistics(fileDir.absolutePath()+"/");

	emit showOverlay("Zapisano", 2000);
}

void ProcessingThread::saveStatistics(QString file){
	
	QString filePath = file.append("stat.txt");
	QFile fileTrj(filePath);

	if (fileTrj.open(QIODevice::WriteOnly)){

		QTextStream in(&fileTrj);
		in << statisticsDialog->getStats();
		fileTrj.close();
	}
}


QString ProcessingThread::saveTrajectory(QString file){
	
	QFile fileTrj(file+"traj.trj");

	if (fileTrj.open(QIODevice::WriteOnly)){

		QTextStream in(&fileTrj);

		// wysokosc i szerokosc obrazu
		in << Settings::instance()->defSize.width;
		in << "\n";
		in << Settings::instance()->defSize.height;
		in << "\n";

		// ilosc punktow
		in << hand[0]->allXYZReal.size();
		in << "\n";

		//for(int k = 0; k < 1; ++k){
		for(int i = 0; i < hand[0]->allXYZReal.size(); ++i){
			
			//if(	hand[0]->allXYZReal[i].x >= -1 && 
			//	hand[0]->allXYZReal[i].y >= -1 &&
			//	hand[0]->allXYZReal[i].z >= -1){

				in << hand[0]->allXYZReal[i].x <<",";
				in << hand[0]->allXYZReal[i].y <<",";
				in << hand[0]->allXYZReal[i].z <<"\n";
			//}
			//qDebug() << hand[0]->allXYZ[i].x;
			//qDebug() << hand[0]->allXYZ[i].y;
			//qDebug() << hand[0]->allXYZ[i].z;
		}

		fileTrj.close();
		return file;
	}
	return "";
}


// reinicjalizacja zawsze zachodzi z mainIdleLoop
// sa juz zainicjalizowane kamery
// inicjalizujemy filmy tylko wtedy, kiedy bedzie VIDEO
bool ProcessingThread::reinitAll(){

	bool all = true;
	processType = Settings::instance()->processType;
	if(processType == VIDEO){
		all = initFilms();
	}

	duringRecognition = false;
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
	numFrames = 0;
	currentBgFrames = 0;

	startProcessTime = 0;
	endProcessTime = 0;

	skinTime = 0;
	counterSkin = 0;
	stereoTime = 0;
	counterPoints = 0;

	framesStartStopCounter = 0;
	framesProcessingCounter = 0;

	stereoAlg = Settings::instance()->stereoAlg;
	segmantationAlg = Settings::instance()->segmantationAlg;

	for(int i = 0; i < bufferSkin[0].size(); ++i){
		cvReleaseImage(&bufferSkin[0][i]);
		cvReleaseImage(&bufferSkin[1][i]);
		cvReleaseImage(&bufferHand[0][i]);
		cvReleaseImage(&bufferHand[1][i]);
		cvReleaseImage(&bufferRectified[0][i]);
		cvReleaseImage(&bufferRectified[1][i]);
		cvReleaseImage(&bufferDepth[i]);
	}
	bufferSkin[0].clear();
	bufferSkin[1].clear();
	bufferHand[0].clear();
	bufferHand[1].clear();
	bufferRectified[0].clear();
	bufferRectified[1].clear();
	bufferDepth.clear();

	return all;
}


void ProcessingThread::reinitFrames(){

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


bool ProcessingThread::reinitModules(){

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

	Settings::instance()->defSize = cvSize(640, 480);

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

void ProcessingThread::setCalibration(bool newVal){
	calibMutex.lock();
	calibration = newVal;
	calibMutex.unlock();
}

bool ProcessingThread::getCalibration(){
	bool toRet;
	calibMutex.lock();
	toRet = calibration;
	calibMutex.unlock();
	return toRet;
}

void ProcessingThread::setNothing(bool newVal){
	nothingMutex.lock();
	nothing = newVal;
	nothingMutex.unlock();
}

bool ProcessingThread::getNothing(){
	bool toRet;
	nothingMutex.lock();
	toRet = nothing;
	nothingMutex.unlock();
	return toRet;
}

// statystyki fps
void ProcessingThread::updateFPS(){
	numFrames++;
	//float currentUpdate = getGameTime();
	if(startTime == 0){
		time(&startTime);
		return;
	}

	time(&endTime);
	fps = numFrames /(difftime(endTime, startTime));
}

void ProcessingThread::initGameTime(){

	startTime = 0;
}

// Called every time you need the current game time
float ProcessingThread::getGameTime(){
	UINT64 ticks;
	float time;
	// This is the number of clock ticks since start
	if( !QueryPerformanceCounter((LARGE_INTEGER *)&ticks) )
	ticks = GetTickCount();//(UINT64)timeGetTime();
	time = (float)(__int64)ticks/(float)(__int64)ticksPerSecond;
	time -= timeAtGameStart;
	return time;
}