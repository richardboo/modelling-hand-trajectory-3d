#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QString>
#include "ui_statisticsdialog.h"

class StatisticsDialog : public QDialog
{
	Q_OBJECT

public:
	StatisticsDialog(QWidget *parent = 0);
	~StatisticsDialog();

	void showStatistics();
	void showNone();
	QString getStats();
	QString getFileName();

	int fps;
	int timeS;

	double timeProcess;
	int framesProcess;
	int framesProper;

	double timeSkin;
	double timeStereo;

	QString file1;
	QString file2;
	QString calibration;
	QString trajectory;
	QString stats;


private:
	Ui::StatisticsDialog ui;
};

#endif // STATISTICSDIALOG_H
