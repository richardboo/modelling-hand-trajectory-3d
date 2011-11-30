#include "mgrstereohand.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MgrStereoHand w;
	w.init();
	w.show();
	return a.exec();
}
