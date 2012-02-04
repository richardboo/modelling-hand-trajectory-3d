#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

#include <QObject>
#include <QMutex>

#include "facedetector.hpp"
#include "skindetector.hpp"
#include "handdetector.hpp"
#include "stereomodule.h"
#include "blob.hpp"
#include "framegrabber.h"
#include "cameradevice.hpp"
#include "videograbber.h"
#include "calibrationmodule.h"
#include "signrecognitionmodule.h"
#include "bgmask.hpp"
#include "framestorage.h"

class ProcessingThread : public QObject
{
	Q_OBJECT

public:
	ProcessingThread(QObject *parent = 0);
	~ProcessingThread();

	void init(FrameStorage * f);
	void setExit();

public slots:
	void process();

signals:
	void finished();
	void showImages();


private:
	void mainLoop();
	void mainIdleLoop();
	void reinit();
	bool initCameras();
	void initModules();
	bool initFilms();

	// przechwytywanie obrazu
	FrameGrabber * frameGrabber[2];

	// algorytmika
	BGMask * bgmask[2];
	SkinDetector * skinDetection[2];
	HandDetector * handDetection[2];
	FaceDetector * faceDetection[2];
	StereoModule * stereoModule;
	CalibrationModule * calibModule;
	SignRecognitionModule * srModule;

	// wykryte bloby
	Blob * hand[2];
	Blob * head[2];

	FrameStorage * fs;

	QMutex stopMutex;
	bool stop;
};

#endif // PROCESSINGTHREAD_H
