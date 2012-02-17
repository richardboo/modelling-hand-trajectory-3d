#include "statisticsdialog.h"
#include "settings.h"
#include "stereomodule.h"
#include "myhandbm.h"
#include "faststereostate.h"

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
	stats += "Stereo dopasowanie (avg): \t" + QString::number(timeStereo) + "ms\n";

	if(Settings::instance()->stereoAlg == MINE_RND_){
		stats += "Znalezionych punktow dopasowania: \t" + QString::number(counterPoints) + "\n";
	}

	stats += "\n..ZAPIS..\n\n";
	stats += "Film1: " + (file1.isEmpty() ? "(brak)" : file1.mid(file1.lastIndexOf("/")+1))+"\n";
	stats += "Film2: " + (file2.isEmpty() ? "(brak)" : file2.mid(file2.lastIndexOf("/")+1))+"\n";
	stats += "Kalibracja: " + (calibration.isEmpty() ? "(brak)" : calibration.mid(calibration.lastIndexOf("/")+1))+"\n";
	stats += "Trajektoria: " + (trajectory.isEmpty() ? "(brak)" : trajectory.mid(trajectory.lastIndexOf("/")+1))+"\n";
	stats += "Probki: " + (this->stats.isEmpty() ? "(brak)" : this->stats )+"\n";

	ui.statLabel->setText(stats);
}

void StatisticsDialog::showNone(){
	ui.statLabel->setText("(brak statystyk)");
}

QString StatisticsDialog::getStats(){
	QString stats = ui.statLabel->text();

	stats += "\n..USTAWIENIA ALGORYTMU STEREO..\n\n";
	switch(Settings::instance()->stereoAlg){
	
		case BM_:

			stats += "SADWindowSize:\t" + QString::number(StereoModule::BMState.SADWindowSize			 ) + "\n";
			stats += "minDisparity:\t" +  QString::number(StereoModule::BMState.minDisparity			 ) + "\n";
			stats += "preFilterCap:\t" + QString::number(StereoModule::BMState.preFilterCap			 ) + "\n";
			stats += "numberOfDisparities:\t" + QString::number(StereoModule::BMState.numberOfDisparities	 ) + "\n";
			stats += "preFilterSize:\t" + QString::number(StereoModule::BMState.preFilterSize			 ) + "\n";
			stats += "uniquenessRatio:\t" + QString::number(StereoModule::BMState.uniquenessRatio		 ) + "\n";
			stats += "textureThreshold:\t" + QString::number(StereoModule::BMState.textureThreshold		 ) + "\n";

			break;

		case SGBM_:

			stats += "SADWindowSize:\t" +		QString::number(StereoModule::sgbm.SADWindowSize		 ) + "\n";
			stats += "minDisparity:\t" +		QString::number(StereoModule::sgbm.minDisparity			 ) + "\n";
			stats += "preFilterCap:\t" +		QString::number(StereoModule::sgbm.preFilterCap			 ) + "\n";
			stats += "numberOfDisparities:\t" + QString::number(StereoModule::sgbm.numberOfDisparities	 ) + "\n";
			stats += "speckleWindowSize:\t" +		QString::number(StereoModule::sgbm.speckleWindowSize	 ) + "\n";
			stats += "speckleRange:\t" +		QString::number(StereoModule::sgbm.speckleRange			 ) + "\n";
			stats += "uniquenessRatio:\t" +	QString::number(StereoModule::sgbm.uniquenessRatio		 ) + "\n";
			stats += "disp12MaxDiff:\t" +	QString::number(StereoModule::sgbm.disp12MaxDiff		 ) + "\n";
			break;

		case BMCUDA_:

			stats += "SADWindowSize:\t" + QString::number(StereoModule::BMStateCuda.SADWindowSize			 ) + "\n";
			stats += "minDisparity:\t" +  QString::number(StereoModule::BMStateCuda.minDisparity			 ) + "\n";
			stats += "preFilterCap:\t" + QString::number(StereoModule::BMStateCuda.preFilterCap			 ) + "\n";
			stats += "numberOfDisparities:\t" + QString::number(StereoModule::BMStateCuda.numberOfDisparities	 ) + "\n";
			stats += "preFilterSize:\t" + QString::number(StereoModule::BMStateCuda.preFilterSize			 ) + "\n";
			stats += "uniquenessRatio:\t" + QString::number(StereoModule::BMStateCuda.uniquenessRatio		 ) + "\n";
			stats += "textureThreshold:\t" + QString::number(StereoModule::BMStateCuda.textureThreshold		 ) + "\n";

			break;

		case MINE_BM_:
			stats += "medianSmooth:\t" +	QString::number(StereoModule::myHandBMState.medianSmooth		) + "\n";
			stats += "numberOfDisparities:\t" +			QString::number(StereoModule::myHandBMState.numberOfDisparities ) + "\n";
			break;

		case MINE_RND_:

			stats += "featuresTheshold:\t" + QString::number(StereoModule::fastState.featuresTheshold ) + "\n";
			stats += "featuresNr:\t" +  QString::number(StereoModule::fastState.featuresNr		 ) + "\n";

			break;
	}


	return stats;
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