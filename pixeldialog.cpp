#include "pixeldialog.h"
#include "stereomodule.h"

PixelDialog::PixelDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

PixelDialog::~PixelDialog()
{

}

void PixelDialog::accept(){

	StereoModule::myHandBMState.medianSmooth		= ui.medianBox->value();
	StereoModule::myHandBMState.numberOfDisparities = ui.dispBox->value();

	QDialog::accept();
}

void PixelDialog::reject(){
	// tu odrzucenie zmian

	ui.medianBox->setValue(StereoModule::myHandBMState.medianSmooth);
	ui.dispBox->setValue(StereoModule::myHandBMState.numberOfDisparities);

	QDialog::reject();
}
