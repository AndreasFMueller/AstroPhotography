/*
 * astrotime.cpp -- Time utility
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroCoordinates.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroFormat.h>
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <string>

namespace astro {
namespace app {
namespace time {

/**
 * \brief Table of options for the astrofocus program
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief Help function
 */
static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] <longitude> <latitude>" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug           enter debug mode"
		<< std::endl;
	std::cout << " -h,--help            display this help message and exit"
		<< std::endl;
}

/**
 * \brief Main function for the astrofocus program
 *
 * \param argc	number of arguments
 * \param argv	vector of arguments
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astrotime");
	debugthreads = 1;
	int	c;
	int	longindex;
	putenv((char *)"POSIXLY_CORRECT=1");    // cast to silence compiler
	while (EOF != (c = getopt_long(argc, argv, "dh?",
		longopts, &longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	if (optind >= argc) {
		std::cerr << "longitude missing" << std::endl;
		return EXIT_FAILURE;
	}
	astro::Angle	longitude(std::stod(argv[optind++]), Angle::Degrees);
	if (optind >= argc) {
		std::cerr << "latitude missing" << std::endl;
		return EXIT_FAILURE;
	}
	astro::Angle	latitude(std::stod(argv[optind++]), Angle::Degrees);
	astro::LongLat	longlat(longitude, latitude);

	astro::AzmAltConverter	azmaltconverter(longlat);
	std::cout << azmaltconverter.LMST().hms() << std::endl;

	return EXIT_SUCCESS;
}

} // namespace time
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::time::main>(argc, argv);
}
