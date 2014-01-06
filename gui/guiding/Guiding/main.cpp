#include <QApplication>
#include <module.hh>
#include <guider.hh>
#include <AstroDebug.h>
#include <cstdlib>
#include <pthread.h>
#include <cassert>
#include <cerrno>
#include <guidingconnectiondialog.h>

int main(int argc, char *argv[]) {
	// unsure we are logging debug messages
	debuglevel = LOG_DEBUG;
	debugtimeprecision = 3;
	debugthreads = 1;

	// now we start Qt initialization
	QApplication a(argc, argv);

	GuidingConnectionDialog	d;
	d.show();

	return a.exec();
}
