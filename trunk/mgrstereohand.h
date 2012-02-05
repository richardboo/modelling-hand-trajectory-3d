#ifndef MGRSTEREOHAND_H
#define MGRSTEREOHAND_H

#include <QtGui/QMainWindow>
#include <QDir>
#include <QString>
#include <QButtonGroup>
#include <QThread>
#include <QWidget>

#include "mywindow.hpp"
#include "ui_mgrstereohand.h"
#include "algorithmsdialog.h"
#include "statisticsdialog.h"
#include "calibrationdialog.h"
#include "drawingmodule.hpp"
#include "processingthread.h"
#include "framestorage.h"

#include "calibrationmodule.h"

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

public slots:
	// funkcja odrysowywania w okienkach
	void showImages();
	void errorMessage(QString title, QString message);
	void startedProcess();
	void finishedProcess();
	void calibrationNotSet();
	void showOverlay(QString text, int time);

private:
	Ui::MgrStereoHandClass ui;

	QButtonGroup * sizeGroup;
	QButtonGroup * processGroup;

	// dialogi
	AlgorithmsDialog * algorithmsDialog;
	StatisticsDialog * statisticsDialog;
	CalibrationDialog * calibDialog;

	QDir lastLoadDir;
	
	// okna
	MyWindow * leftCamWindow;
	MyWindow * rightCamWindow;
	MyWindow * disparityWindow;
	MyWindow * trajectoryWindow;

	// rysowanie na oknach
	DrawingModule * drawingModule;

	// przetwarzanie
	QThread * realProcessingThead;
	ProcessingThread * process;
	FrameStorage * fs;
	CalibrationModule * calibModule;

	void initUI();
	void initWindows();

	QString loadFilm(int id);
	void setUIstartEnabled(bool enable);

private slots:

	void loadFilmClicked0();
	void loadFilmClicked1();

	void loadCalibrationFile();

	void startProcess();
	void stopProcess();

	void realExit();

	void processTypeChanged(int id);

	void changeShowImage(int value);
	void sizeButtonClicked(int id);
	void calibrateButtonClicked();

	void exit();

	void startCalibrationClickedFromDialog();
	void savedCalibrationFromDialog();
};

#endif // MGRSTEREOHAND_H
