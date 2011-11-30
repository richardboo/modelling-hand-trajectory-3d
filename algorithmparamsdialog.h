#ifndef ALGORITHMPARAMSDIALOG_H
#define ALGORITHMPARAMSDIALOG_H

#include <QDialog>
#include "ui_algorithmparamsdialog.h"

class AlgorithmParamsDialog : public QDialog
{
	Q_OBJECT

public:
	AlgorithmParamsDialog(QWidget *parent = 0);
	~AlgorithmParamsDialog();

private:
	Ui::AlgorithmParamsDialog ui;
};

#endif // ALGORITHMPARAMSDIALOG_H
