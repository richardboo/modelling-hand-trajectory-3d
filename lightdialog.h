#ifndef LIGHTDIALOG_H
#define LIGHTDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QStringList>
#include <QAbstractButton>
#include <QList>

#include "ui_lightdialog.h"

/**
 * Okno dialogowe wyboru rodzaju swiatla na przetwarzanych filmach (tylko do celow statystycznych).
 */
class LightDialog : public QDialog
{
	Q_OBJECT

public:
	LightDialog(QWidget *parent = 0);
	~LightDialog();

private:
	Ui::LightDialog ui;

	QButtonGroup * group;

	void initUI();

public slots:
	void accept();
	void reject();
};

#endif // LIGHTDIALOG_H
