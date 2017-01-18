#include "mainwindow.h"
#include <QApplication>
#include <AstroDebug.h>
#include <taskconnectiondialog.h>

int main(int argc, char *argv[]) {
	// setup for logging
	debuglevel = LOG_DEBUG;
	debugtimeprecision = 3;
	debugthreads = 1;

	// now start application initialization
	QApplication a(argc, argv);

	// open connection dialgo
	TaskConnectionDialog	d;
	d.show();

	// run the application
	return a.exec();
}
