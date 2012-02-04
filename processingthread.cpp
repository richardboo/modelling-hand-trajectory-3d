#include "processingthread.h"
#include "settings.h"

#include <QDebug>
#include <QThread>
#include <QWaitCondition>

ProcessingThread::ProcessingThread(QObject *parent)
	: QObject(parent)
{
	frameGrabber[0] = frameGrabber[1] = NULL;
	srModule = new SignRecognitionModule("A.xml");

	hand[0] = hand[1] = head[0] = head[0] = NULL;

	stop = false;
}

ProcessingThread::~ProcessingThread()
{

}

void ProcessingThread::init(FrameStorage * f){

	fs = f;

	initCameras();
	initModules();
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
        /////////////////////////////////

		// jesli sa kamery - pobranie obrazu
		if(frameGrabber[0] != NULL && frameGrabber[1] != NULL){

			bool newOne = false;
			for(int i = 0; i < 1/*ONE*/; ++i){
				if(frameGrabber[i]->hasNextFrame()){
					// TODO jesli jest video to zatrzymanie
					// i zebranie statystyk
					fs->frame[i] = frameGrabber[i]->getNextFrame();
					newOne = true;
				}
			}
			if(!newOne)
				continue;

			cvResize(fs->frame[0], fs->frameSmaller[0]);//, CV_INTER_NN);
			cvResize(fs->frame[1], fs->frameSmaller[1]);//, CV_INTER_NN);
			emit showImages();
		}

	}
	emit finished();
}

void ProcessingThread::mainLoop(){

}

void ProcessingThread::mainIdleLoop(){

}

void ProcessingThread::reinit(){
	
}

bool ProcessingThread::initCameras(){
	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		delete frameGrabber[1];
	}

	CameraDevice * cam1 = new CameraDevice(0);
	CameraDevice * cam2 = cam1;
	// ONE
	//CameraDevice * cam2 = new CameraDevice(1);

	// ONE
	cam1->init(Settings::instance()->defSize.width,
				   Settings::instance()->defSize.height);

/*
	if(!cam1->init(Settings::instance()->defSize.width,
				   Settings::instance()->defSize.height) || 
	   !cam2->init(Settings::instance()->defSize.width,
	   Settings::instance()->defSize.height)){
		frameGrabber[0] = NULL;
		frameGrabber[1] = NULL;
		return false;
	}
*/

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
	calibModule = new CalibrationModule();
}

bool ProcessingThread::initFilms(){
	if(frameGrabber[0] != NULL){
		delete frameGrabber[0];
		delete frameGrabber[1];
	}

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

void ProcessingThread::setExit(){
    // Stop thread //
    /////////////////////////////////
    stopMutex.lock();
	stop = true;
    stopMutex.unlock();
    /////////////////////////////////
}