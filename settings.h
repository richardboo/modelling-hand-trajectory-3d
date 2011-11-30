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
#define HIST_ 2
#define ALL_ 3
#define ONLY_BG_ 4
#define BGSKIN_ 5

#define BM_ 0
#define SGBM_ 1
#define BMCUDA_ 2
#define MINE_ 3
#define MINE_BM_ 4
#define MINE_RND_ 5


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

	QSize & getSize()		{ return size; }

	int getImageWidth()		{ return size.width(); }
	int getImageHeight()	{ return size.height(); }

	int getLight()			{ return light; }
	int getBkg()			{ return bkg; }

	// settery
	void setImageType(int newOne)		{ imageType = newOne; qDebug() << "im type: " << newOne; }
	void setSize(int id);

	void setSegmantationAlg(int newOne)	{ segmantationAlg = newOne; qDebug() << "segm: " << newOne; }
	void setStereoAlg(int newOne)		{ stereoAlg = newOne; qDebug() << "stereo: " << newOne;}
	void setLight(int newOne)			{ light = newOne; qDebug() << "lisght: " << newOne;}
	void setBkg(int newOne)				{ bkg = newOne; qDebug() << "bkg: " << newOne;}

	void initImageType(int current);
	void initSize(int current);
	void initSegmentation(QStringList list, int current);
	void initLight(QStringList list, int current);
	void initBkg(QStringList list, int current);
	void initStereo(QStringList list, int current);

	QString getImageTypeString();
	QString getLightString();
	QString getBkgString();

	QString getSegmantationString();
	QString getStereoString();

	CvSize imageSize;

	QDir lastLoadCalibDir;

	CvSize defSize;
	CvSize defSmallSize;
	

	int segmantationAlg;
	int stereoAlg;
	int light;
	int bkg;

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

