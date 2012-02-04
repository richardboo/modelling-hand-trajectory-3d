#include "mgrstereohand.h"

#include <QFileDialog>
#include <QDebug>
#include <QThread>

MgrStereoHand::MgrStereoHand(QWidget *parent, Qt::WFlags flags)	: QMainWindow(parent)
{
	ui.setupUi(this);
	initUI();
}

MgrStereoHand::~MgrStereoHand(){

	delete leftCamWindow;
	delete rightCamWindow;
	delete disparityWindow;
	delete trajectoryWindow;
	delete fs;
	delete drawingModule;
}

bool MgrStereoHand::init(){
	initWindows();

	drawingModule = new DrawingModule;

	fs = new FrameStorage;
	fs->initFrames();

	process = new ProcessingThread;
	process->init(fs);
	realProcessingThead = new QThread();
	process->moveToThread(realProcessingThead);

	connect(realProcessingThead, SIGNAL(started()), process, SLOT(process()));
	connect(process, SIGNAL(finished()), realProcessingThead, SLOT(quit()));
	connect(process, SIGNAL(finished()), process, SLOT(deleteLater()));
	connect(realProcessingThead, SIGNAL(finished()), realProcessingThead, SLOT(deleteLater()));
	
	connect(realProcessingThead, SIGNAL(finished()), this, SLOT(realExit()));
	connect(process, SIGNAL(showImages()), this, SLOT(showImages()));
	
	realProcessingThead->start();

	return true;
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
	Settings::instance()->processType = processGroup->checkedId();

	
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

	switch(Settings::instance()->imageType){
		case 0:
			leftCamWindow->showImage(fs->frameSmaller[0]);
			rightCamWindow->showImage(fs->frameSmaller[1]);
		break;
		/*
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
	}*/
	}
}

void MgrStereoHand::loadFilmClicked0(){
	loadFilm(0);
}

void MgrStereoHand::loadFilmClicked1(){
	loadFilm(1);
}

QString MgrStereoHand::loadFilm(int id){

	QString file = QFileDialog::getOpenFileName(NULL, tr("Wybierz film"), lastLoadDir.absolutePath(), tr("Filmy (*.avi)"));
	if(file != NULL){
		lastLoadDir = QDir(file);

		if(id == 0){
			ui.labelFilm0->setText(lastLoadDir.relativeFilePath(lastLoadDir.dirName()));
			Settings::instance()->fileFilm0 = file;
		}else{
			ui.labelFilm1->setText(lastLoadDir.relativeFilePath(lastLoadDir.dirName()));
			Settings::instance()->fileFilm1 = file;
		}

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
		/*
		QByteArray ba = file.toAscii();
		calibModule->calibrationLoad(ba.data());
		*/
	}
}

void MgrStereoHand::savedCalibrationFromDialog(){
	ui.labelCalibrationFile->setText( Settings::instance()->lastLoadCalibDir.relativeFilePath( Settings::instance()->lastLoadCalibDir.dirName()));
	Settings::instance()->fileCalib = Settings::instance()->lastLoadCalibDir.absolutePath();
}

void MgrStereoHand::startProcess(){
	//nothing = false;
	qDebug() << "cos robie" << QThread::currentThread()->objectName();
}

void MgrStereoHand::stopProcess(){
	//nothing = true;
}

void MgrStereoHand::changeShowImage(int value){
	Settings::instance()->setImageType(value);
	ui.labelShowImage->setText(Settings::instance()->getImageTypeString());
}

void MgrStereoHand::sizeButtonClicked(int id){
	Settings::instance()->setSize(id);
}

void MgrStereoHand::processTypeChanged(int id){
	//processType = id;
	ui.calibrateButton->setEnabled(id != VIDEO);
}

void MgrStereoHand::setUIstartEnabled(bool enable){
	ui.buttonStart->setEnabled(!enable);
	ui.groupBoxCalibration->setEnabled(!enable);
	ui.groupBoxSize->setEnabled(!enable);
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
	qDebug() << "start calib";
	//calibration = true;
	//nothing = false;
}
