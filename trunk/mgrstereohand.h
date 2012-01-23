#ifndef MGRSTEREOHAND_H
#define MGRSTEREOHAND_H

#include <QtGui/QMainWindow>
#include <QDir>
#include <QString>
#include <QTimer>

#include <vector>

#include "facedetector.hpp"
#include "skindetector.hpp"
#include "handdetector.hpp"
#include "mywindow.hpp"
#include "stereomodule.h"

#include "ui_mgrstereohand.h"

#include "algorithmsdialog.h"
#include "statisticsdialog.h"
#include "calibrationdialog.h"
#include "blob.hpp"
#include "drawingmodule.hpp"

#include "framegrabber.h"
#include "cameradevice.hpp"
#include "videograbber.h"
#include "calibrationmodule.h"

#include "signrecognitionmodule.h"

#include "bgmask.hpp"


#define VIDEO	0
#define CAMERA	2

#define STATE_BEFORE_HIST 0
#define STATE_AFTER_HIST 1

#define STATE_BEFORE_BACK 0
#define STATE_BACK 1
#define STATE_AFTER_BACK 2


class MgrStereoHand : public QMainWindow
{
	Q_OBJECT

public:
	MgrStereoHand(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MgrStereoHand();

	bool init();

private:
	Ui::MgrStereoHandClass ui;

	// typ przetwarzania video/camera/record
	int processType;
	bool calibration;
	bool nothing;

	int stateHist;
	int stateBack;

	// do nagrywania filmow
	//CvVideoWriter * videoWriter[2];

	// timer glownej petli
	QTimer * framesTimer;

	QButtonGroup * sizeGroup;
	QButtonGroup * processGroup;

	QString fileFilm0;
	QString fileFilm1;
	QString fileCalib;

	// dialogi
	AlgorithmsDialog * algorithmsDialog;
	StatisticsDialog * statisticsDialog;
	CalibrationDialog * calibDialog;

	QDir lastLoadDir;
	
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

	// stan rozpoznania pobierania trajektorii dla kazdego z obrazow
	// rozpoczecie, kiedy startRecognized[0/1] == 1
	// jesli jedna klatke nie rozpozna - przechodzi na 2
	// zakonczenie, kiedy startRecognized[0/1] == 3
	// po kazdej petli obydwa == min z nich
	int startRecognized[2];
	bool lastStart[2];

	// wykryte dlonie
	Blob * hand[2];
	Blob * head[2];

	// okna
	MyWindow * leftCamWindow;
	MyWindow * rightCamWindow;
	MyWindow * disparityWindow;
	MyWindow * trajectoryWindow;

	// rysowanie na oknach
	DrawingModule * drawingModule;

	void initUI();
	QString loadFilm(int id);

	bool checkIfStart();
	void setUIstartEnabled(bool enable);


	// inicjalizacja
	bool initWindows();
	bool initCameras();
	void initFrames();
	bool initFilms();

	// reinicjalizacja - nowe przetwarzanie
	// wywoluje wszystkie funkcje ponizej, aby odpowiednio zainicjalizowac elementy do przetwarzania
	bool reinitAll();
	
	bool reinitModules();
	void reinitFrames();

	void makeEverythingStart();
	void makeEverythingStop();
	
	// funkcja odrysowywania w okienkach
	void showImages();

	// zachowanie trajektorii
	QString saveTrajectory(QString file);
	void saveStatistics(QString file);

	// nagrywanie filmow
	void recordFilms();
	void storeStatistics();
	// bufory do nagrywania
	std::vector<IplImage *> buffer[2];


	///////////////////// statystyki
	float getGameTime();
	void initGameTime();
	void updateFPS();
	
	UINT64 ticksPerSecond;
	//////////////////////

private slots:

	// glowna funkcja przetwarzania
	void mainLoop();
	void mainIdleLoop();

	void loadFilmClicked0();
	void loadFilmClicked1();

	void loadCalibrationFile();

	void startProcess();
	void stopProcess();

	void processTypeChanged(int id);

	void changeShowImage(int value);
	void sizeButtonClicked(int id);
	void calibrateButtonClicked();

	void exit();

	void startCalibrationClickedFromDialog();
	void savedCalibrationFromDialog();
};

#endif // MGRSTEREOHAND_H
