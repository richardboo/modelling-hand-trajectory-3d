#ifndef ALGORITHMSDIALOG_H
#define ALGORITHMSDIALOG_H

#include <QDialog>
#include <QButtonGroup>

#include "ui_algorithmsdialog.h"

#include "lightdialog.h"
#include "backgrounddialog.h"
#include "settings.h"

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

	QButtonGroup * stereoGroup;
	QButtonGroup * segmGroup;


	void initUI();

private slots:
	void showLightDialogClicked();
	void showBkgDialogClicked();

	void segmButtonClicked(int id);
	void stereoButtonClicked(int id);

	void lightChanged();
	void bkgChanged();
};

#endif // ALGORITHMSDIALOG_H
