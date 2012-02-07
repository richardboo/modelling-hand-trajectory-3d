#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include "ui_calibrationdialog.h"

#include "calibrationmodule.h"

class CalibrationDialog : public QDialog
{
	Q_OBJECT

public:
	CalibrationDialog(QWidget *parent = 0);
	~CalibrationDialog();

	void init(CalibrationModule * module);
	//void endCalibration();

	int getBoardSize1(){	return boardSize1;	}
	int getBoardSize2(){	return boardSize2;	}
	int getSampleCounter(){	return sampleCounter;	}

public slots:
	void endCalibration();


private:
	Ui::CalibrationDialog ui;

	void initUI();
	

	int boardSize1, boardSize2;
	int sampleCounter;
	CalibrationModule * calibModule;
	

private slots:
	void buttonCalibrationStartClicked();
	void buttonCalibrationSaveClicked();

	void boardSizeChanged1(int newOne);
	void boardSizeChanged2(int newOne);
	void samplesCounterChanged(int newOne);

signals:
	void calibrationStartFromDialog();
	void calibrationSaved();
};

#endif // CALIBRATIONDIALOG_H
