#include "bmdialog.h"

#include "stereomodule.h"

BMDialog::BMDialog(QWidget *parent, bool cuda): QDialog(parent)
{
	ui.setupUi(this);

	isCuda = cuda;

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

}

BMDialog::~BMDialog()
{

}

void BMDialog::accept(){

	if(!isCuda){
		StereoModule::BMState.SADWindowSize = ui.sadBox->value();
		StereoModule::BMState.minDisparity = ui.minDispBox->value();
		StereoModule::BMState.preFilterCap = ui.preCapBox->value();
		StereoModule::BMState.numberOfDisparities = ui.dispBox->value();
		StereoModule::BMState.preFilterSize = ui.preSizeBox->value();
		StereoModule::BMState.uniquenessRatio = ui.uniqueBox->value();
		StereoModule::BMState.textureThreshold = ui.textureBox->value();
	}
	else{
		StereoModule::BMStateCuda.SADWindowSize = ui.sadBox->value();
		StereoModule::BMStateCuda.minDisparity = ui.minDispBox->value();
		StereoModule::BMStateCuda.preFilterCap = ui.preCapBox->value();
		StereoModule::BMStateCuda.numberOfDisparities = ui.dispBox->value();
		StereoModule::BMStateCuda.preFilterSize = ui.preSizeBox->value();
		StereoModule::BMStateCuda.uniquenessRatio = ui.uniqueBox->value();
		StereoModule::BMStateCuda.textureThreshold = ui.textureBox->value();
	}

	QDialog::accept();
}

void BMDialog::reject(){
	// tu odrzucenie zmian

	if(isCuda){
		ui.sadBox->setValue(StereoModule::BMStateCuda.SADWindowSize);
		ui.minDispBox->setValue(StereoModule::BMStateCuda.minDisparity);
		ui.preCapBox->setValue(StereoModule::BMStateCuda.preFilterCap);
		ui.dispBox->setValue(StereoModule::BMStateCuda.numberOfDisparities);
		ui.preSizeBox->setValue(StereoModule::BMStateCuda.preFilterSize);
		ui.uniqueBox->setValue(StereoModule::BMStateCuda.uniquenessRatio);
		ui.textureBox->setValue(StereoModule::BMStateCuda.textureThreshold);
	}
	else{
		ui.sadBox->setValue(StereoModule::BMState.SADWindowSize);
		ui.minDispBox->setValue(StereoModule::BMState.minDisparity);
		ui.preCapBox->setValue(StereoModule::BMState.preFilterCap);
		ui.dispBox->setValue(StereoModule::BMState.numberOfDisparities);
		ui.preSizeBox->setValue(StereoModule::BMState.preFilterSize);
		ui.uniqueBox->setValue(StereoModule::BMState.uniquenessRatio);
		ui.textureBox->setValue(StereoModule::BMState.textureThreshold);
	}
	

	QDialog::reject();
}