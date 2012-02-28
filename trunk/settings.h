#pragma once

#include <QSize>
#include <QObject>
#include <QStringList>
#include <QMutex>
#include <QString>

#include <QDebug>
#include <QDir>

#include <cv.h>

#define RESULT_FAIL 0
#define RESULT_OK 1

#define RGB_ 0
#define HSV_ 1
#define YCRCB_ 2
#define HIST_ 3
#define ALL_ 4
#define ONLY_BG_ 5
//#define BGSKIN_ 6


#define BM_ 0
#define SGBM_ 1
#define BMCUDA_ 2
#define MINE_ 3
#define MINE_BM_ 4
#define MINE_RND_ 5

#define NONE 3
#define OCCLUSION 2
#define FACE 1
#define OK_FACE_HAND 0

#define VIDEO 2
#define CAMERA 0

class Settings
{

public:
	static Settings* instance()
    {
        static QMutex mutex;
        if (!m_Instance){
            mutex.lock();
             if (!m_Instance)
                m_Instance = new Settings;
             mutex.unlock();
        }
        return m_Instance;
    }

	static void drop(){
        static QMutex mutex;
        mutex.lock();
        delete m_Instance;
        m_Instance = 0;
        mutex.unlock();
    }

	~Settings(void);

	// gettery
	int imageType;

	int getImageWidth()		{ return size.width(); }
	int getImageHeight()	{ return size.height(); }

	int getLight()			{ return light; }
	int getBkg()			{ return bkg; }

	// settery
	void setImageType(int newOne)		{ imageType = newOne; }

	void setSegmantationAlg(int newOne)	{ segmantationAlg = newOne; }
	void setStereoAlg(int newOne)		{ stereoAlg = newOne;}
	void setLight(int newOne)			{ light = newOne; }
	void setBkg(int newOne)				{ bkg = newOne; }

	void initImageType(int current);
	void initSegmentation(QStringList list, int current);
	void initLight(QStringList list, int current);
	void initBkg(QStringList list, int current);
	void initStereo(QStringList list, int current);

	QString getImageTypeString();
	QString getLightString();
	QString getBkgString();

	QString getSegmantationString();
	QString getStereoString();

	//CvSize imageSize;

	QDir lastLoadCalibDir;
	QDir lastLoadDir;

	CvSize defSize;
	CvSize defSmallSize;
	

	int segmantationAlg;
	int stereoAlg;
	int light;
	int bkg;

	bool manual;

	bool changeTrajectory;

	QString fileFilm0;
	QString fileFilm1;
	QString fileCalib;

	int processType;

private:
	Settings(void);
	
    static Settings* m_Instance;

	QSize size;

	QStringList imageTypes;
	QStringList segmantations;
	QStringList stereo;
	QStringList lights;
	QStringList bkgs;
};

