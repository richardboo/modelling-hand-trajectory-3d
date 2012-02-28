#ifndef PIXELDIALOG_H
#define PIXELDIALOG_H

#include <QDialog>
#include "ui_pixeldialog.h"

/**
 * Okno dialogowe zmian parametrow algorytmu Pixel Matching.
 */
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
