#include "mgrstereohand.h"

#include <QFileDialog>
#include <QDebug>
#include <QThread>
#include <QByteArray>
#include <QMessageBox>

MgrStereoHand::MgrStereoHand(QWidget *parent, Qt::WFlags flags)	: QMainWindow(parent)
{
	ui.setupUi(this);
	initUI();
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
}

MgrStereoHand::~MgrStereoHand(){

	delete leftCamWindow;
	delete rightCamWindow;
	delete disparityWindow;
	delete trajectoryWindow;
	delete fs;
	delete drawingModule;
	delete calibModule;
}

bool MgrStereoHand::init(){
	initWindows();

	drawingModule = new DrawingModule;

	fs = new FrameStorage;
	fs->initFrames();

	calibModule = new CalibrationModule;

	process = new ProcessingThread;
	process->init(fs, calibModule, calibDialog, statisticsDialog);

	calibDialog->init(calibModule);

	realProcessingThead = new QThread();
	process->moveToThread(realProcessingThead);

	connect(realProcessingThead, SIGNAL(started()), process, SLOT(process()));
	connect(process, SIGNAL(finished()), realProcessingThead, SLOT(quit()));
	connect(process, SIGNAL(finished()), process, SLOT(deleteLater()));
	connect(realProcessingThead, SIGNAL(finished()), realProcessingThead, SLOT(deleteLater()));
	
	connect(realProcessingThead, SIGNAL(finished()), this, SLOT(realExit()));
	
	connect(process, SIGNAL(showImages()), this, SLOT(showImages()));
	connect(process, SIGNAL(messageError(QString, QString)), this, SLOT(errorMessage(QString, QString)));
	connect(process, SIGNAL(startedProcess()), this, SLOT(startedProcess()));
	connect(process, SIGNAL(finishedProcess()), this, SLOT(finishedProcess()));
	connect(process, SIGNAL(calibrationNotSet()), this, SLOT(calibrationNotSet()));
	connect(process, SIGNAL(showOverlay(QString,int)), this, SLOT(showOverlay(QString,int)));
	connect(process, SIGNAL(getFilmFileName()), this, SLOT(getFilmFileName()));
	//connect(this, SIGNAL(showOverlay(QString,int)), this, SLOT(showOverlay(QString,int)));
	
	realProcessingThead->start();

	return true;
}

void MgrStereoHand::initUI(){

	// rozmiar przetwarzania

	processGroup = new QButtonGroup(this);
	processGroup->addButton(ui.processButtonRadio0, VIDEO);
	//processGroup->addButton(ui.processButtonRadio1, RECORD);
	processGroup->addButton(ui.processButtonRadio2, CAMERA);
	Settings::instance()->processType = processGroup->checkedId();


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

void MgrStereoHand::initWindows(){
	IplImage * black = cvCreateImage(cvSize(320,240), 8, 1);
	cvZero(black);

	leftCamWindow = new MyWindow("kamera 1");
	leftCamWindow->setXY(350, 180);
	leftCamWindow->showImage(black);

	rightCamWindow = new MyWindow("kamera 2");
	rightCamWindow->setXY(670, 180);
	rightCamWindow->showImage(black);

	disparityWindow = new MyWindow("mapa glebokosci");
	disparityWindow->setXY(350, 490);
	disparityWindow->showImage(black);

	trajectoryWindow = new MyWindow("trajektoria");
	trajectoryWindow->setXY(670, 490);
	trajectoryWindow->showImage(black);
}

// funkcja odrysowywania w okienkach

void MgrStereoHand::showImages(){

	if(process->getNothing()){
		cvResize(fs->frameShow[0], fs->frameSmaller[0]);//, CV_INTER_NN);
		cvResize(fs->frameShow[1], fs->frameSmaller[1]);//, CV_INTER_NN);
		//drawingModule->drawFPSonFrame(process->fps > 0 ? process->fps : 0, fs->frameSmaller[0]);
		leftCamWindow->showImage(fs->frameSmaller[0]);
		rightCamWindow->showImage(fs->frameSmaller[1]);
		trajectoryWindow->showImage(fs->trajectorySmaller);
		disparityWindow->showImage(fs->disparityToShow);
		return;
	}

	switch(Settings::instance()->imageType){

		case 0:

			cvResize(fs->frameShow[0], fs->frameSmaller[0]);//, CV_INTER_NN);
			cvResize(fs->frameShow[1], fs->frameSmaller[1]);//, CV_INTER_NN);
			drawingModule->drawFPSonFrame(process->fps > 0 ? process->fps : 0, fs->frameSmaller[0]);
			leftCamWindow->showImage(fs->frameSmaller[0]);
			rightCamWindow->showImage(fs->frameSmaller[1]);
			break;
			
		case 1:

			cvResize(fs->frameRectified[0], fs->frameSmaller[0], CV_INTER_NN);
			cvResize(fs->frameRectified[1], fs->frameSmaller[1], CV_INTER_NN);
			drawingModule->drawFPSonFrame(process->fps > 0 ? process->fps : 0, fs->frameSmaller[0]);

			for(int i = 0; i < 2; ++i){
				if(process->hand[i]->lastRect.x > -1 ){
					drawingModule->drawSmallerRectOnFrame(process->hand[i]->lastRect, fs->frameSmaller[i], cvScalar(200,0,0));
					//drawingModule->drawSmallerRectOnFrame(head[0]->lastRect, frameSmaller[0], cvScalar(0,200,0));
				}
				if(process->head[i]->lastRect.x > -1){
					drawingModule->drawSmallerRectOnFrame(process->head[i]->lastRect, fs->frameSmaller[i], cvScalar(0,200,0));
				}
			}
			leftCamWindow->showImage(fs->frameSmaller[0]);
			rightCamWindow->showImage(fs->frameSmaller[1]);
			break;
		case 2:
			cvResize(fs->frameSkin[0], fs->frameSmallerGray[0], CV_INTER_NN);
			cvResize(fs->frameSkin[1], fs->frameSmallerGray[1], CV_INTER_NN);
			drawingModule->drawFPSonFrame(process->fps > 0 ? process->fps : 0, fs->frameSmaller[0]);
			leftCamWindow->showImage(fs->frameSmallerGray[0]);
			rightCamWindow->showImage(fs->frameSmallerGray[1]);
			break;
		case 3:
			/*
			if(process->hand[0]->lastRect.x != -1){
				drawingModule->drawTextOnFrame("znaleziono", fs->frameBlob[0]);
				drawingModule->drawMiddleOnFrame(process->hand[0], fs->frameBlob[0]);
			}
			if(process->hand[1]->lastRect.x != -1){
				drawingModule->drawTextOnFrame("znaleziono", fs->frameBlob[1]);
				drawingModule->drawMiddleOnFrame(process->hand[1], fs->frameBlob[1]);
			}*/
			cvZero(fs->frameShow[0]);
			cvZero(fs->frameShow[1]);

			cvCopy(fs->frameRectified[0], fs->frameShow[0], fs->frameBlob[0]);
			cvCopy(fs->frameRectified[1], fs->frameShow[1], fs->frameBlob[1]);
			cvResize(fs->frameShow[0], fs->frameSmaller[0], CV_INTER_NN);
			cvResize(fs->frameShow[1], fs->frameSmaller[1], CV_INTER_NN);
			drawingModule->drawFPSonFrame(process->fps > 0 ? process->fps : 0, fs->frameSmaller[0]);
			leftCamWindow->showImage(fs->frameSmaller[0]);
			rightCamWindow->showImage(fs->frameSmaller[1]);
			
			break;
	}
	//cvResize(fs->trajectory, fs->trajectorySmaller, CV_INTER_NN);
	//cvResize(fs->disparityToShow, fs->disparitySmaller, CV_INTER_NN);
	trajectoryWindow->showImage(fs->trajectorySmaller);
	//STEREO
	disparityWindow->showImage(fs->disparitySmaller);
}

void MgrStereoHand::loadFilmClicked0(){
	loadFilm(0);
}

void MgrStereoHand::loadFilmClicked1(){
	loadFilm(1);
}

QString MgrStereoHand::loadFilm(int id){

	QString file = QFileDialog::getOpenFileName(NULL, tr("Wybierz film"), Settings::instance()->lastLoadDir.absolutePath(), tr("Filmy (*.avi)"));
	if(file != NULL){
		Settings::instance()->lastLoadDir = QDir(file);

		if(id == 0){
			ui.labelFilm0->setText(Settings::instance()->lastLoadDir.relativeFilePath(Settings::instance()->lastLoadDir.dirName()));
			Settings::instance()->fileFilm0 = file;
		}else{
			ui.labelFilm1->setText(Settings::instance()->lastLoadDir.relativeFilePath(Settings::instance()->lastLoadDir.dirName()));
			Settings::instance()->fileFilm1 = file;
		}

		//qDebug() << Settings::instance()->fileFilm0 << Settings::instance()->fileFilm1;

		ui.processButtonRadio0->click();
	}
	return file;
}

void MgrStereoHand::loadCalibrationFile(){
	QString file = QFileDialog::getOpenFileName(NULL, tr("Wybierz dane kalibracji"),  Settings::instance()->lastLoadCalibDir.absolutePath(), tr("Kalibracja (*.calib)"));
	if(file != NULL){
		 Settings::instance()->lastLoadCalibDir = QDir(file);
		ui.labelCalibrationFile->setText( Settings::instance()->lastLoadCalibDir.relativeFilePath( Settings::instance()->lastLoadCalibDir.dirName()));
		Settings::instance()->fileCalib = file;
		QByteArray ba = file.toAscii();
		calibModule->calibrationLoad(ba.data());
	}
}

void MgrStereoHand::savedCalibrationFromDialog(){
	ui.labelCalibrationFile->setText( Settings::instance()->lastLoadCalibDir.relativeFilePath( Settings::instance()->lastLoadCalibDir.dirName()));
	Settings::instance()->fileCalib = Settings::instance()->lastLoadCalibDir.absolutePath();
}

void MgrStereoHand::startProcess(){
	process->setNothing(false);
}                                               

void MgrStereoHand::stopProcess(){
	process->setNothing(true);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
}

void MgrStereoHand::changeShowImage(int value){
	Settings::instance()->setImageType(value);
	ui.labelShowImage->setText(Settings::instance()->getImageTypeString());
}


void MgrStereoHand::processTypeChanged(int id){
	Settings::instance()->processType = id;
	ui.calibrateButton->setEnabled(id != VIDEO);
}

void MgrStereoHand::setUIstartEnabled(bool enable){
	ui.buttonStart->setEnabled(!enable);
	ui.groupBoxCalibration->setEnabled(!enable);
	ui.groupBoxFilm->setEnabled(!enable);
	ui.buttonStop->setEnabled(enable);
}

void MgrStereoHand::calibrateButtonClicked(){
	// przeprowadzenie kalibracji
	calibDialog->exec();
}

void MgrStereoHand::exit(){
	process->setExit();
}

void MgrStereoHand::realExit(){
	QApplication::exit(0);
}

void MgrStereoHand::startCalibrationClickedFromDialog(){
	//qDebug() << "start calib";
	process->setCalibration(true);
	process->setNothing(false);
}

void MgrStereoHand::errorMessage(QString title, QString message){
	QMessageBox::information(NULL, title, message);
}

void MgrStereoHand::startedProcess(){
	setUIstartEnabled(true);
}

void MgrStereoHand::finishedProcess(){
	setUIstartEnabled(false);
}

void MgrStereoHand::calibrationNotSet(){
	ui.labelCalibrationFile->setText("plik nie zapisany");
}

void MgrStereoHand::showOverlay(QString text, int time){
	displayOverlay(leftCamWindow->name, text.toStdString(), time);
	displayOverlay(rightCamWindow->name, text.toStdString(), time);
}

void MgrStereoHand::getFilmFileName(){
	QString file1 = QFileDialog::getSaveFileName(NULL, tr("Zapisz film"), Settings::instance()->lastLoadDir.absolutePath(), tr("Filmy (*.avi)"));
	process->recordFilms(file1);
}