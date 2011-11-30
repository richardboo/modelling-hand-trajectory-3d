#include "statisticsdialog.h"
#include "settings.h"

#include <QString>

StatisticsDialog::StatisticsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
}

StatisticsDialog::~StatisticsDialog()
{

}

void StatisticsDialog::showStatistics(){

	QString stats = "Algorytm segmentacji:\t" + Settings::instance()->getSegmantationString() + "\n";
	stats += "Algorytm stereo dopasowania:\t" + Settings::instance()->getStereoString() + "\n\n";

	stats += "Oswietlenie:\t" + Settings::instance()->getLightString()+"\n";
	stats += "Tlo:\t" + Settings::instance()->getBkgString()+"\n\n";

	stats += "FPS:\t" + QString::number(fps) + "\n";
	stats += "KBps:\t" + QString::number(bps/1024) + "\n";

	stats += "Ilosc klatek:\t" + QString::number(allFrames) + "\n";
	stats += "Ilosc danych:\t" + QString::number(allBits/1024)+"KB\n";
	//stats += "Dlugosc filmu:\t" + QString::number(timeS)+"\n\n";

	stats += "\nZapis:\n";
	stats += "Film1: " + (file1.isEmpty() ? "(brak)" : file1)+"\n\n";
	stats += "Film2: " + (file2.isEmpty() ? "(brak)" : file2)+"\n\n";
	stats += "Kalibracja: " + (calibration.isEmpty() ? "(brak)" : calibration)+"\n\n";
	stats += "Trajektoria: " + (trajectory.isEmpty() ? "(brak)" : trajectory)+"\n\n";

	ui.statLabel->setText(stats);
}

void StatisticsDialog::showNone(){
	ui.statLabel->setText("(brak statystyk)");
}