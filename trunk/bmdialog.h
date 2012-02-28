#ifndef BMDIALOG_H
#define BMDIALOG_H

#include <QDialog>
#include "ui_bmdialog.h"

/**
 * Okno dialogowe zmian parametrow algorytmu BM.
 */
class BMDialog : public QDialog
{
	Q_OBJECT

public:
	BMDialog(QWidget *parent = 0, bool cuda = false);
	~BMDialog();

private:
	Ui::BMDialog ui;
	bool isCuda;

public slots:
	void accept();
	void reject();
};

#endif // BMDIALOG_H
