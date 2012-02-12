#ifndef PIXELDIALOG_H
#define PIXELDIALOG_H

#include <QDialog>
#include "ui_pixeldialog.h"

class PixelDialog : public QDialog
{
	Q_OBJECT

public:
	PixelDialog(QWidget *parent = 0);
	~PixelDialog();

private:
	Ui::PixelDialog ui;

public slots:
	void accept();
	void reject();
};

#endif // PIXELDIALOG_H
