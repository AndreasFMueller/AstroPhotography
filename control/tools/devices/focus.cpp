/*
 * focus.cpp -- command line focus utility
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <iostream>
#include <AstroFormat.h>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <AstroIO.h>
#include <AstroUtils.h>
#include <includes.h>

using namespace astro;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;
using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::io;

namespace astro {
namespace app {
namespace focus {

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ]" << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug               increase debug level"
		<< std::endl;
	std::cout << "    -h,-?,--help             show this help message and "
		"exit" << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,	'd' }, /* 2 */
{ "config",	required_argument,	NULL,	'c' },
{ "help",	no_argument,		NULL,	'h' }, /* 5 */
{ NULL,		0,			NULL,	 0  }, /* 6 */
};

/**
 * \brief main function for the focus program
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;

	while (EOF != (c = getopt_long(argc, argv, "dhc",
		longopts, &longindex)))
		switch (c) {
		case 'c':
			
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}

	// next argument must be the focuser url
	if (argc <= optind) {
		throw std::runtime_error("focuser url missing");
	}
	std::string	focusername(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser: %s", focusername.c_str());

	// get the camera
	Repository	repository;
	Devices	devices(repository);
	FocuserPtr	focuser = devices.getFocuser(focusername);
	if (!focuser) {
		debug(LOG_ERR, DEBUG_LOG, 0, "focuser not found");
		throw std::runtime_error("no such device");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a focuser");

	// next argument must be the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);
	if (command == "get") {
		unsigned short	min = focuser->min();
		unsigned short	max = focuser->max();
		unsigned short	current = focuser->current();
		std::cout << "min:     " << min << std::endl;
		std::cout << "max:     " << max << std::endl;
		std::cout << "current: " << current << std::endl;
	}
	if (command == "set") {
		if (argc <= optind) {
			std::runtime_error("position argument missing");
		}
		std::string	positionstring(argv[optind++]);
		unsigned short	position = std::stoi(positionstring);
		focuser->set(position);
	}
	
	return EXIT_SUCCESS;
}

} // namespace focus
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::focus::main>(argc, argv);
}
