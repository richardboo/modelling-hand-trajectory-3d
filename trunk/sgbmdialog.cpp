#include "sgbmdialog.h"

#include "stereomodule.h"

SGBMDialog::SGBMDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

SGBMDialog::~SGBMDialog()
{

}

void SGBMDialog::accept(){
	// tu sprawdzenie ktory jest wybrany

	StereoModule::sgbm.SADWindowSize = ui.sadBox->value();
	StereoModule::sgbm.minDisparity = ui.minDispBox->value();
	StereoModule::sgbm.preFilterCap = ui.preCapBox->value();
	StereoModule::sgbm.numberOfDisparities = ui.dispBox->value();
	StereoModule::sgbm.speckleWindowSize = ui.speckleSizeBox->value();
	StereoModule::sgbm.speckleRange = ui.speckleBox->value();
	StereoModule::sgbm.uniquenessRatio = ui.uniqueBox->value();
	StereoModule::sgbm.disp12MaxDiff = ui.maxDiffBox->value();

	QDialog::accept();
}

void SGBMDialog::reject(){
	// tu odrzucenie zmian

	ui.sadBox->setValue(StereoModule::sgbm.SADWindowSize);
	ui.minDispBox->setValue(StereoModule::sgbm.minDisparity);
	ui.preCapBox->setValue(StereoModule::sgbm.preFilterCap);
	ui.dispBox->setValue(StereoModule::sgbm.numberOfDisparities);
	ui.speckleSizeBox->setValue(StereoModule::sgbm.speckleWindowSize);
	ui.speckleBox->setValue(StereoModule::sgbm.speckleRange);
	ui.uniqueBox->setValue(StereoModule::sgbm.uniquenessRatio);
	ui.maxDiffBox->setValue(StereoModule::sgbm.disp12MaxDiff);
	
	QDialog::reject();
}