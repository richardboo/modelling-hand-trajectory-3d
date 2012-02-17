#include "calibrationdialog.h"
#include "settings.h"

#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QByteArray>

CalibrationDialog::CalibrationDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	initUI();
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
}

CalibrationDialog::~CalibrationDialog(){

}


void CalibrationDialog::initUI(){

	connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(accept()));

	connect(ui.buttonCalibrationStart,	SIGNAL(clicked()),
			this,						SLOT(buttonCalibrationStartClicked()));

	connect(ui.buttonCalibrationSave,	SIGNAL(clicked()),
			this,						SLOT(buttonCalibrationSaveClicked()));


	connect(ui.spinBoxCalibrationBoard1,	SIGNAL(valueChanged(int)),
			this,							SLOT(boardSizeChanged1(int)));

	connect(ui.spinBoxCalibrationBoard2,	SIGNAL(valueChanged(int)),
			this,							SLOT(boardSizeChanged2(int)));

	connect(ui.spinBoxCalibrationSamples,	SIGNAL(valueChanged(int)),
			this,							SLOT(samplesCounterChanged(int)));

	connect(ui.realSizeSpinBox,				SIGNAL(valueChanged(double)),
			this,							SLOT(realSizeChanged(double)));

	

	boardSize1 = ui.spinBoxCalibrationBoard1->value();
	boardSize2 = ui.spinBoxCalibrationBoard2->value();
	sampleCounter = ui.spinBoxCalibrationSamples->value();
	realSize = ui.realSizeSpinBox->value();
}

void CalibrationDialog::init(CalibrationModule * module){
	calibModule = module;

	connect(calibModule, SIGNAL(calibrationEnded()), this, SLOT(endCalibration()));
}


void CalibrationDialog::buttonCalibrationStartClicked(){
	
	ui.buttonCalibrationStart->setEnabled(false);
	ui.buttonCalibrationSave->setEnabled(false);
	ui.spinBoxCalibrationBoard1->setEnabled(false);
	ui.spinBoxCalibrationBoard2->setEnabled(false);
	ui.spinBoxCalibrationSamples->setEnabled(false);
	ui.closeButton->setEnabled(false);

	ui.labelCalibrationStatus->setText(tr("Kalibracja rozpoczêta"));

	emit calibrationStartFromDialog();
}

void CalibrationDialog::buttonCalibrationSaveClicked(){
	QString file = QFileDialog::getSaveFileName(NULL, tr("Zapisz kalibracje do pliku..."), Settings::instance()->lastLoadCalibDir.absolutePath(), tr("Kalibracja (*.calib)"));
	if(file != NULL){
		Settings::instance()->lastLoadCalibDir = QDir(file);
		QByteArray ba = file.toAscii();
		calibModule->calibrationSave(ba.data());
		ui.labelCalibrationStatus->setText(tr("Kalibracja zapisana"));

		emit calibrationSaved();
	}
}

void CalibrationDialog::samplesCounterChanged(int newOne){
	sampleCounter = newOne;
}

void CalibrationDialog::endCalibration(){

	ui.buttonCalibrationStart->setEnabled(true);
	ui.buttonCalibrationSave->setEnabled(true);
	ui.spinBoxCalibrationBoard1->setEnabled(true);
	ui.spinBoxCalibrationBoard2->setEnabled(true);
	ui.spinBoxCalibrationSamples->setEnabled(true);
	ui.closeButton->setEnabled(true);

	ui.labelCalibrationStatus->setText(tr("Kalibracja zakonczona pomyslnie"));
}

void CalibrationDialog::boardSizeChanged1(int newOne){
	boardSize1 = newOne;
}

void CalibrationDialog::boardSizeChanged2(int newOne){
	boardSize2 = newOne;
}

void CalibrationDialog::realSizeChanged(double newOne){
	realSize = newOne;
}