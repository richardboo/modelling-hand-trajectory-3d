#include "statisticsdialog.h"
#include "settings.h"

#include <QString>
#include <QDateTime>
#include <QDir>
#include <QFile>

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

	QString stats = "..SRODOWISKO..\n\n";
		
	stats += "Algorytm segmentacji:\t" + Settings::instance()->getSegmantationString() + "\n";
	stats += "Algorytm stereo dopasowania:\t" + Settings::instance()->getStereoString() + "\n\n";

	stats += "Oswietlenie:\t" + Settings::instance()->getLightString()+"\n";
	stats += "Tlo:\t" + Settings::instance()->getBkgString()+"\n\n";

	//stats += "FPS:\t" + QString::number(fps) + "\n\n";

	stats += "..BADANIE POBIERANIA TRAJEKTORII..\n\n";

	stats += "Przetwarzanie: \t" + QString::number(timeProcess) + "s\n";
	stats += "Klatki przetworzone: \t" + QString::number(framesProcess) + "\n";
	stats += "Poprawnie przetworzone: \t" + QString::number(framesProper) +"\n";
	if(framesProper > 0)
		stats += "Procent poprawny: \t" + QString::number(framesProper*100/framesProcess) +"%\n\n";
	else
		stats += "Procent poprawny: \t0%\n\n" ;

	stats += "Segmentacja dloni (avg): \t" + QString::number(timeSkin) + "ms\n";
	stats += "Stereo dopasowanie (avg): \t" + QString::number(timeStereo) + "ms\n\n";

	stats += "..ZAPIS..\n\n";
	stats += "Film1: " + (file1.isEmpty() ? "(brak)" : file1.mid(file1.lastIndexOf("/")+1))+"\n";
	stats += "Film2: " + (file2.isEmpty() ? "(brak)" : file2.mid(file2.lastIndexOf("/")+1))+"\n";
	stats += "Kalibracja: " + (calibration.isEmpty() ? "(brak)" : calibration.mid(calibration.lastIndexOf("/")+1))+"\n";
	stats += "Trajektoria: " + (trajectory.isEmpty() ? "(brak)" : trajectory.mid(trajectory.lastIndexOf("/")+1))+"\n";
	stats += "Probki: " + (stats.isEmpty() ? "(brak)" : stats )+"\n";

	ui.statLabel->setText(stats);
}

void StatisticsDialog::showNone(){
	ui.statLabel->setText("(brak statystyk)");
}

QString StatisticsDialog::getStats(){
	return ui.statLabel->text();
}

QString StatisticsDialog::getFileName(){

	QDateTime dateTime = QDateTime::currentDateTime();
	QString dateTimeString = dateTime.toString(Qt::ISODate);
	dateTimeString = dateTimeString.replace(10, 1, "_");
	dateTimeString = dateTimeString.replace(QString(":"), QString("_"));
	QString str;
	if(!Settings::instance()->fileFilm0.isEmpty()){
		str = Settings::instance()->fileFilm0;
		str.chop(5);
	}
	else{
		str = "cam";
	}

	//QString str = Settings::instance()->fileFilm0.isEmpty()?"":Settings::instance()->fileFilm0;
	return str.append("_s").append(QString().number( Settings::instance()->segmantationAlg)).append("_a").append(QString().number(Settings::instance()->stereoAlg)).append("_").append(dateTimeString);

}