/*
 * imagedisplaytest.cpp -- main function for the snowgui application
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <imagedisplaywidget.h>
#include <QApplication>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroDiscovery.h>
#include <AstroIO.h>
#include <getopt.h>
#include <CommunicatorSingleton.h>

namespace snowgui {

/**
 * \brief Usage function for the snowgui program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage:" << std::endl;
	std::cout << "    " << path.basename() << " [ options ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -c,--config=<cfg>   use configuration in file <cfg>"
		<< std::endl;
	std::cout << "  -d,--debug          increase debug level" << std::endl;
	std::cout << "  -h,-?,--help        show this help message and exit"
		<< std::endl;
}

/**
 * \brief command line options
 */
static struct option	longopts[] = {
{ "config",		required_argument,	NULL,	'c' },
{ "debug",		no_argument,		NULL,	'd' },
{ "help",		no_argument,		NULL,	'h' },
{ NULL,			0,			NULL,	 0  }
};

/**
 * \brief Main-function of the snowgui program
 */
int main(int argc, char *argv[]) {
	// debug initialization
	debug_set_ident("snowgui");
	debugthreads = 1;

	// Ice initialization
	snowstar::CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh?",
		longopts, &longindex)))
		switch (c) {
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	
	// next argument must be a fits file name
	if (argc <= optind) {
		throw std::runtime_error("missing fits file argument");
	} 

	debug(LOG_DEBUG, DEBUG_LOG, 0, "test program starting up");

	// start the application
	QApplication a(argc, argv);
	a.setApplicationDisplayName(QString("Test"));

	// create a new 
	while (optind < argc) {
		std::string	filename(argv[optind++]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open file %s",
			filename.c_str());
		astro::io::FITSin	infile(filename);
		astro::image::ImagePtr	image = infile.read();
		snowgui::imagedisplaywidget	*w = new imagedisplaywidget;
		w->setWindowTitle(QString(filename.c_str()));
		w->show();
		w->setImage(image);
	}
	return a.exec();
}

} // namespace snowgui

// wrapper used to catch and log any exceptions
int	main(int argc, char *argv[]) {
	return astro::main_function<snowgui::main>(argc, argv);
}
