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
	int bps;
	int allBits;
	int allFrames;
	int timeS;

	QString file1;
	QString file2;
	QString calibration;
	QString trajectory;

private:
	Ui::StatisticsDialog ui;
};

#endif // STATISTICSDIALOG_H
