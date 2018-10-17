/*
 * snowlife.cpp -- main function for the lifeview application
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <browserwindow.h>
#include <QApplication>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroIO.h>
#include <getopt.h>
#include <QFileDialog>
#include <imagedisplaywidget.h>
#include "liveview.h"

namespace snowgui {
namespace snowlife {

/**
 * \brief Usage function for the snowgui program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage:" << std::endl;
	std::cout << "    " << path.basename() << " [ options ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug          increase debug level" << std::endl;
	std::cout << "  -h,-?,--help        show this help message and exit"
		<< std::endl;
}

/**
 * \brief command line options
 */
static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,	'd' },
{ "help",		no_argument,		NULL,	'h' },
{ NULL,			0,			NULL,	 0  }
};

/**
 * \brief Main-function of the snowgui program
 */
int main(int argc, char *argv[]) {
	// debug initialization
	debug_set_ident("snowlife");
	debugthreads = 1;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break; case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test program starting up");

	// start the application
	QApplication a(argc, argv);
	a.setApplicationDisplayName(QString("Snowlife"));

	// create a liveview main window
	LiveView	liveview;
	liveview.show();

	return a.exec();
}

} // namespace snowlife
} // namespace snowgui

// wrapper used to catch and log any exceptions
int	main(int argc, char *argv[]) {
	return astro::main_function<snowgui::snowlife::main>(argc, argv);
}
