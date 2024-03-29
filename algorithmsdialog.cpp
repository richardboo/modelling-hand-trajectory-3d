#include "algorithmsdialog.h"

#include <QStringList>
#include <QList>
#include <QAbstractButton>


AlgorithmsDialog::AlgorithmsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
	initUI();
}

AlgorithmsDialog::~AlgorithmsDialog(){

}

void AlgorithmsDialog::initUI(){

	lightDialog = new LightDialog(this);
	backgroundDialog = new BackgroundDialog(this);

	sgbmDialog = new SGBMDialog(this);
	bmDialog = new BMDialog(this, false);
	cudaDialog = new BMDialog(this, true);
	fastDialog = new FastDialog(this);
	pixelDialog = new PixelDialog(this);

	connect(ui.buttonGetLight,	SIGNAL(clicked()),
			this,				SLOT(showLightDialogClicked()));

	connect(lightDialog,		SIGNAL(accepted()),
			this,				SLOT(lightChanged()));

	connect(ui.buttonGetBkg,	SIGNAL(clicked()),
			this,				SLOT(showBkgDialogClicked()));

	connect(backgroundDialog,	SIGNAL(accepted()),
			this,				SLOT(bkgChanged()));


	connect(ui.bmParamsButton,	SIGNAL(clicked()),
			this,				SLOT(showBMParamsClicked()));
	
	connect(ui.sgbmParamsButton,	SIGNAL(clicked()),
			this,				SLOT(showSGBMParamsClicked()));
	
	//connect(ui.cudaParamsButton,	SIGNAL(clicked()),
	//		this,				SLOT(showCUDAParamsClicked()));

	connect(ui.pixelParamsButton,	SIGNAL(clicked()),
		this,				SLOT(showPixelParamsClicked()));
	connect(ui.fastParamsButton,	SIGNAL(clicked()),
		this,				SLOT(showfastParamsClicked()));


	connect(sgbmDialog,	SIGNAL(accepted()),
			this,				SLOT(bmChanged()));

	connect(bmDialog,	SIGNAL(accepted()),
			this,				SLOT(sgbmChanged()));

	connect(cudaDialog,	SIGNAL(accepted()),
			this,				SLOT(cudaChanged()));

	connect(ui.startStopTrajectoryButton,	SIGNAL(clicked()),
		this,					SLOT(startStopTrajectoryClicked()));

	connect(ui.manualCheckBox,	SIGNAL(stateChanged (int)),
			this,				SLOT(manualChanged(int)));

	Settings::instance()->manual = (ui.manualCheckBox->checkState() == Qt::Checked);

	
	// algorytmy stereo dopasowania
	stereoGroup = new QButtonGroup(this);
	stereoGroup->addButton(ui.radioButtonStereo0, 0);
	stereoGroup->addButton(ui.radioButtonStereo1, 1);
	//stereoGroup->addButton(ui.radioButtonStereo2, 2);
	stereoGroup->addButton(ui.radioButtonStereo3, 3);
	stereoGroup->addButton(ui.radioButtonStereo4, 4);
	stereoGroup->addButton(ui.radioButtonStereo5, 5);
	
	QStringList stereoList;
	QList<QAbstractButton *> list = stereoGroup->buttons();
	for(int i = 0; i < list.size(); ++i)
		stereoList << list[i]->text();

	Settings::instance()->initStereo(stereoList, stereoGroup->checkedId());
	connect(stereoGroup,		SIGNAL(buttonClicked (int)),
			this,				SLOT(stereoButtonClicked(int)));

	// algorytmy segmentacji
	segmGroup = new QButtonGroup(this);
	segmGroup->addButton(ui.radioButtonSegm0, 0);
	segmGroup->addButton(ui.radioButtonSegm1, 1);
	segmGroup->addButton(ui.radioButtonSegm2, 2);
	segmGroup->addButton(ui.radioButtonSegm3, 3);
	segmGroup->addButton(ui.radioButtonSegm4, 4);
	segmGroup->addButton(ui.radioButtonSegm5, 5);
	
	QStringList segmList;
	QList<QAbstractButton *> list2 = segmGroup->buttons();
	for(int i = 0; i < list2.size(); ++i)
		segmList << list2[i]->text();

	Settings::instance()->initSegmentation(segmList, segmGroup->checkedId());
	connect(segmGroup,			SIGNAL(buttonClicked (int)),
			this,				SLOT(segmButtonClicked(int)));

	ui.labelLight->setText(Settings::instance()->getLightString());
	ui.labelBkg->setText(Settings::instance()->getBkgString());
}

void AlgorithmsDialog::showLightDialogClicked(){
	lightDialog->exec();
}

void AlgorithmsDialog::showBkgDialogClicked(){
	backgroundDialog->exec();
}

void AlgorithmsDialog::segmButtonClicked(int id){
	Settings::instance()->setSegmantationAlg(id);
}

void AlgorithmsDialog::stereoButtonClicked(int id){
	Settings::instance()->setStereoAlg(id);
}

void AlgorithmsDialog::lightChanged(){
	ui.labelLight->setText(Settings::instance()->getLightString());
}
void AlgorithmsDialog::bkgChanged(){
	ui.labelBkg->setText(Settings::instance()->getBkgString());
}

void AlgorithmsDialog::showBMParamsClicked(){
	bmDialog->exec();
}

void AlgorithmsDialog::showSGBMParamsClicked(){
	sgbmDialog->exec();
}

void AlgorithmsDialog::showCUDAParamsClicked(){
	cudaDialog->exec();
}

void AlgorithmsDialog::showPixelParamsClicked(){
	pixelDialog->exec();
}

void AlgorithmsDialog::showfastParamsClicked(){
	fastDialog->exec();
}

void AlgorithmsDialog::bmChanged(){
	
}

void AlgorithmsDialog::sgbmChanged(){
	
}

void AlgorithmsDialog::cudaChanged(){

}

void AlgorithmsDialog::startStopTrajectoryClicked(){
	Settings::instance()->changeTrajectory = true;
}

void AlgorithmsDialog::manualChanged(int newOne){
	Settings::instance()->manual = (newOne == Qt::Checked);
	ui.startStopTrajectoryButton->setEnabled(Settings::instance()->manual);
}