#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

#include <QObject>
#include <QMutex>
#include <vector>

#include <windows.h>
#include <mmsystem.h>
#include <omp.h>
#include <time.h>

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
#include "calibrationdialog.h"
#include "drawingmodule.hpp"
#include "statisticsdialog.h"

// pomocnicze stany przetwarzania/inicjalizacji dla algorytmow wykorzystujacy histogram lub elimiacje tla
#define STATE_BEFORE_HIST 0
#define STATE_AFTER_HIST 1

#define STATE_BEFORE_BACK 0
#define STATE_BACK 1
#define STATE_AFTER_BACK 2

/**
 * Glowny watek przetwarzania.
 * Realizowany w dwoch trybach: 
 *   - idle - kiedy nie jest rozpoczete przetwarzania, wysiwetla obraz z kamer (jesli sa dostepne),
 *   - main - nastepuje przetwarzanie obrazow z wybranym algorytmem stereo dopasowania i algorytmem rozpoznawania skory
 * Przetwarzanie w glownej petli obejmuje:
 *   - pobranie obrazow,
 *   - rektyfikacje obrazow,
 *   - inicjalizacje specyficznych cech algorytmow (update histogramu twarzy lub obliczenie i uaktualnienie tla),
 *   - wykrywanie skory na obrazach (rownolegle na dwoch procesorach, jesli sa dostepne),
 *   - segmentacja dloni (rownolegle),
 *   - rozpoznawanie znaku rozpoczecia/zakonczenia pobierania trajektorii (rownolegle)
 *   - stereo dopasowanie, jesli przetwarzanie jest w fazie pobierania trajektorii
 *   - wyliczenie prawdziwej odleglosci od kamery
 */
class ProcessingThread : public QObject
{
	Q_OBJECT

public:
	ProcessingThread(QObject *parent = 0);
	~ProcessingThread();

	void init(	FrameStorage * f, 
				CalibrationModule * calibMod, 
				CalibrationDialog * calibDial,
				StatisticsDialog * statDial);
	void setExit();

	void setCalibration(bool newVal);
	bool getCalibration();

	void setNothing(bool newVal);
	bool getNothing();

	float fps;
	// wykryte bloby
	Blob * hand[2];
	Blob * head[2];

public slots:
	void process();
	void recordFilms(QString file1);

signals:
	void finished();
	void showImages();
	void messageError(QString title, QString str);
	void startedProcess();
	void finishedProcess();
	void calibrationNotSet();
	void showOverlay(QString text, int time);
	void getFilmFileName();

private:
	bool mainLoop();
	bool mainIdleLoop();

	bool initCameras();
	void initModules();
	bool initFilms();
	void initFrames();

	bool checkIfStart();
	void makeEverythingStart();
	void makeEverythingStop();

	// reinicjalizacja
	bool reinitAll();
	void reinitFrames();
	bool reinitModules();
	

	// zapis
	
	void storeStatistics();
	void saveStatistics(QString file);
	QString saveTrajectory(QString file);


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

	// do zebrania info o kalibracji
	CalibrationDialog * calibDialog;
	StatisticsDialog * statisticsDialog;
	DrawingModule * drawingModule;

	FrameStorage * fs;

	// zatrzymanie watku
	QMutex stopMutex;
	bool stop;

	// rozpoczecie/zakonczenie kalibracji
	QMutex calibMutex;
	bool calibration;

	// rozpoczecie/zakoczenie main loop'a
	QMutex nothingMutex;
	bool nothing;
	bool prevNothing;

	// zmienne pomocnicze przetwarzania
	CvRect rect;
	// typ przetwarzania video/camera
	int processType;
	int stateHist;
	int stateBack;
	int currentBgFrames;
	
	// stan rozpoznania pobierania trajektorii dla kazdego z obrazow
	// rozpoczecie, kiedy startRecognized[0/1] == 1
	// jesli jedna klatke nie rozpozna - przechodzi na 2
	// zakonczenie, kiedy startRecognized[0/1] == 3
	// po kazdej petli obydwa == min z nich
	int startRecognized[2];
	bool lastStart[2];


	//////////////// STATYSTYKI ///////////////
	// fps
	// liczenie klatek itd
	long framesCounter;

	bool duringRecognition;

	// rozpoczecie 
	time_t startTime;
	// zakonczenie 
	time_t endTime;

	// czas rozpoczecia i zakonczenia przetwarzania
	time_t startProcessTime;
	time_t endProcessTime;

	// czas: rektyfikacja, wykrywania skory, dopasowanie stereo
	double skinTime;
	int counterSkin;
	double stereoTime;
	int counterPoints;

	// zmienne pomocnicze do liczenia czasu
	time_t tempStart;
	time_t tempEnd;

	// ilosc wszystkich klatek od rozpoczecia pobierania trajektorii do zakonczenia
	int framesStartStopCounter;
	// ilosc wszystkich klatek, w ktorych pobrana byla trajektoria
	int framesProcessingCounter;

	// wektory probek obrazu podczas przetwarzania, pobierane co 10 klatek
	std::vector<IplImage *> bufferSkin[2];
	std::vector<IplImage *> bufferHand[2];
	std::vector<IplImage *> bufferRectified[2];
	std::vector<IplImage *> bufferDepth;

	float lastUpdate;
	float fpsUpdateInterval;
	int  numFrames;
	float timeAtGameStart;
	UINT64 ticksPerSecond;

	float getGameTime();
	void initGameTime();
	void updateFPS();
	
	// bufory do nagrywania
	std::vector<IplImage *> buffer[2];

};

#endif // PROCESSINGTHREAD_H
