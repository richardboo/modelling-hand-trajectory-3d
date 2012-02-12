#include "fastdialog.h"
#include "stereomodule.h"

FastDialog::FastDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

FastDialog::~FastDialog()
{

}

void FastDialog::accept(){

	StereoModule::fastState.featuresTheshold = ui.threshBox->value();
	StereoModule::fastState.featuresNr			= ui.featNrBox->value();

	QDialog::accept();
}

void FastDialog::reject(){
	// tu odrzucenie zmian

	ui.threshBox->setValue(StereoModule::fastState.featuresTheshold);
	ui.featNrBox->setValue(StereoModule::fastState.featuresNr);

	QDialog::reject();
}
