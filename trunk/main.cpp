#include "mgrstereohand.h"
#include "settings.h"
#include <QtGui/QApplication>

/**
 * Glowna funckja apliakcji Qt.
 */
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Settings::instance()->defSize = cvSize(640, 480);
	Settings::instance()->defSmallSize = cvSize(320, 240);
	MgrStereoHand w;
	w.init();
	w.show();
	return a.exec();
}
