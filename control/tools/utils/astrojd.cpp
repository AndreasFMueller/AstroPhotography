/*
 * astrojd.cpp -- Time utility
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
#include <string.h>
#include <time.h>

namespace astro {
namespace app {
namespace time {

/**
 * \brief Table of options for the astrojd program
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ "time",	required_argument,	NULL,		't' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief Help function
 */
static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ]" << std::endl;
	std::cout << std::endl;
	std::cout << "compute julian date time" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug           enter debug mode"
		<< std::endl;
	std::cout << " -h,--help            display this help message and exit"
		<< std::endl;
	std::cout << " -t,--time=<t>        compute julian date for time <t> "
		"in the format " << std::endl;
	std::cout << "                      '%Y-%m-%d %H:%M:%S'" << std::endl;
}

/**
 * \brief Main function for the astrojd program
 *
 * \param argc	number of arguments
 * \param argv	vector of arguments
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astrojd");
	debugthreads = 1;
	time_t	t;
	::time(&t);
	int	c;
	int	longindex;
	putenv(strdup("POSIXLY_CORRECT=1"));
	while (EOF != (c = getopt_long(argc, argv, "dh?t:",
		longopts, &longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 't':
			struct tm tm;
			if (NULL == strptime(optarg, "%F %T", &tm)) {
				std::cerr << "cannot parse date: '" << optarg;
				std::cerr << "'" << std::endl;
				return EXIT_FAILURE;
			}
			t = mktime(&tm);
			break;
		}
	}

	// compute julian date for this time
	JulianDate	jd(t);
	std::cout << stringprintf("%.6f", jd.T()) << std::endl;

	return EXIT_SUCCESS;
}

} // namespace time
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main_function<astro::app::time::main>(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "astrojd filed due to exception: " << x.what();
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}
