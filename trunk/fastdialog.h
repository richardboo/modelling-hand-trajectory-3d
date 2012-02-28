#ifndef FASTDIALOG_H
#define FASTDIALOG_H

#include <QDialog>
#include "ui_fastdialog.h"

/**
 * Okno dialogowe zmian parametrow algorytmu FAST Feature Matching.
 */
class FastDialog : public QDialog
{
	Q_OBJECT

public:
	FastDialog(QWidget *parent = 0);
	~FastDialog();

private:
	Ui::FastDialog ui;

public slots:
	void accept();
	void reject();
};

#endif // FASTDIALOG_H
