#ifndef ALGORITHMSDIALOG_H
#define ALGORITHMSDIALOG_H

#include <QDialog>
#include <QButtonGroup>

#include "ui_algorithmsdialog.h"

#include "lightdialog.h"
#include "backgrounddialog.h"
#include "settings.h"
#include "sgbmdialog.h"
#include "bmdialog.h"

class AlgorithmsDialog : public QDialog
{
	Q_OBJECT

public:
	AlgorithmsDialog(QWidget *parent = 0);
	~AlgorithmsDialog();

private:
	Ui::AlgorithmsDialog ui;

	LightDialog * lightDialog;
	BackgroundDialog * backgroundDialog;

	SGBMDialog * sgbmDialog;
	BMDialog * bmDialog;
	BMDialog * cudaDialog;

	QButtonGroup * stereoGroup;
	QButtonGroup * segmGroup;


	void initUI();

private slots:
	void showLightDialogClicked();
	void showBkgDialogClicked();

	void showBMParamsClicked();
	void showSGBMParamsClicked();
	void showCUDAParamsClicked();

	void startStopTrajectoryClicked();

	void segmButtonClicked(int id);
	void stereoButtonClicked(int id);

	void lightChanged();
	void bkgChanged();

	void bmChanged();
	void sgbmChanged();
	void cudaChanged();
};

#endif // ALGORITHMSDIALOG_H
