#include "settings.h"


Settings* Settings::m_Instance = NULL;

Settings::Settings(void){
	lastLoadCalibDir = QDir("");
	lastLoadDir = QDir("");

	defSize = cvSize(640, 480);
	defSmallSize = cvSize(320, 240);

	changeTrajectory = false;
	processType = 0;

	fileFilm0 = "";
	fileFilm1 = "";
	fileCalib = "";

	manual = true;
}

Settings::~Settings(void)
{
}


void Settings::initImageType(int current){
	imageTypes << "Obraz z kamery"<<"Skalibrowany obraz" << "Wykrywanie skory" << "Znaleziona dlon";
	imageType = current;
}

QString Settings::getImageTypeString(){
	return imageTypes.at(imageType);
}


void Settings::initSegmentation(QStringList list, int current){
	segmantations = list;
	segmantationAlg = current;
}

void Settings::initLight(QStringList list, int current){
	lights = list;
	light = current;
}

void Settings::initBkg(QStringList list, int current){
	bkgs = list;
	bkg = current;
}

QString Settings::getLightString(){
	return lights.at(light);
}

QString Settings::getBkgString(){
	return bkgs.at(bkg);
}

QString Settings::getSegmantationString(){
	return segmantations.at(segmantationAlg);
}
QString Settings::getStereoString(){
	return stereo.at(stereoAlg);
}

void Settings::initStereo(QStringList list, int current){
	stereo = list;
	stereoAlg = current;
}

