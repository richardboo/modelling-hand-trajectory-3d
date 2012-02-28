#ifndef SGBMDIALOG_H
#define SGBMDIALOG_H

#include <QDialog>
#include "ui_sgbmdialog.h"

/**
 * Okno dialogowe zmian parametrow algorytmu SGBM.
 */
class SGBMDialog : public QDialog
{
	Q_OBJECT

public:
	SGBMDialog(QWidget *parent = 0);
	~SGBMDialog();

private:
	Ui::SGBMDialog ui;

public slots:
	void accept();
	void reject();
};

#endif // SGBMDIALOG_H
