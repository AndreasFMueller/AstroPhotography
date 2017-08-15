/*
 * main.cpp -- main function for the snowgui application
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "mainwindow.h"
#include "serverselectiondialog.h"
#include "Application.h"
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroDiscovery.h>
#include <getopt.h>
#include <CommunicatorSingleton.h>
#include <QFile>
#include <QString>

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
	std::cout << "  -s,--server=<s>     name of the server" << std::endl;
	std::cout << "  -q,-qss=<style>     use the stylesheet named <style>"
		<< std::endl;
}

/**
 * \brief command line options
 */
static struct option	longopts[] = {
{ "config",		required_argument,	NULL,	'c' },
{ "debug",		no_argument,		NULL,	'd' },
{ "help",		no_argument,		NULL,	'h' },
{ "server",		required_argument,	NULL,	's' },
{ "qss",		required_argument,	NULL,	'q' },
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

	// if the servername is set, then we don't need to select the server
	// we can start right into the mainwindow
	std::string	servername;

	// parse the command line
	int	c;
	int	longindex;
	char	*qssfilename = NULL;
	while (EOF != (c = getopt_long(argc, argv, "c:dh?s:q:",
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
		case 's':
			servername = std::string(optarg);
			break;
		case 'q':
			qssfilename = optarg;
			break;
		}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "snowgui starting up");

	// start the application
	Application a(argc, argv);
	a.setApplicationDisplayName(QString("SnowGUI"));

	// load the style sheet
	if (qssfilename) {
		QFile	stylefile(qssfilename);
		stylefile.open(QFile::ReadOnly);
		QString	styleSheet = QLatin1String(stylefile.readAll());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "style sheet: %s",
			styleSheet.toLatin1().data());
		a.setStyleSheet(styleSheet);
	}

	// get the service discovery object
	astro::discover::ServiceDiscoveryPtr	servicediscovery
		= astro::discover::ServiceDiscovery::get();
	servicediscovery->start();

	// decide how start: if the servername is set, then we start with
	// the main window configured to talk to the server, otherwise
	// we start with the server selection dialog
	if (servername.size() > 0) {
		astro::discover::ServiceKey	key
			= servicediscovery->waitfor(servername);
		astro::discover::ServiceObject	serviceobject
			= servicediscovery->find(key);
		MainWindow	w(NULL, serviceobject);
		w.show();
		return a.exec();
	} else {
		ServerSelectionDialog	*s
			= new ServerSelectionDialog(NULL, servicediscovery);
		s->exec();
		delete s;
		return a.exec();
	}
	return EXIT_FAILURE;
}

} // namespace snowgui

// wrapper used to catch and log any exceptions
int	main(int argc, char *argv[]) {
	return astro::main_function<snowgui::main>(argc, argv);
}
