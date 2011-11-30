#ifndef BACKGROUNDDIALOG_H
#define BACKGROUNDDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QStringList>
#include <QAbstractButton>
#include <QList>

#include "ui_backgrounddialog.h"

class BackgroundDialog : public QDialog
{
	Q_OBJECT

public:
	BackgroundDialog(QWidget *parent = 0);
	~BackgroundDialog();


private:
	Ui::BackgroundDialog ui;

	QButtonGroup * group;

	void initUI();

public slots:
	void accept();
	void reject();
};

#endif // BACKGROUNDDIALOG_H
