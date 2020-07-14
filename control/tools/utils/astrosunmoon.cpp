/*
 * astroephemeris.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroSolarsystem.h>
#include <stdexcept>
#include <getopt.h>

namespace astro {
namespace app {
namespace ephemeris {

/**
 * \brief Help function
 *
 * \param progname	name of the program
 */
static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] <names> ... " << std::endl;
	std::cout << std::endl;
	std::cout << "compute ephemeris for the named solar system objects";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug         enter debug mode" << std::endl;
	std::cout << " -h,-?,--help       show this help message and exit"
		<< std::endl;
	std::cout << " -t,--time=<t>      compute positions for time <t> in "
		"the format" << std::endl;
	std::cout << "                    '%Y-%m-%d %H:%M:%S'" << std::endl;
}

/**
 * \brief Table of options for the astroephemeris program
 */
static struct option	options[] = {
{ "debug",		no_argument,			NULL,		'd' },
{ "help",		no_argument,			NULL,		'h' },
{ "time",		required_argument,		NULL,		't' },
{ "revolutions",	no_argument,			NULL,		'r' },
{ NULL,			0,				NULL,		 0  }
};

/**
 * \brief Main function for the astroephemeris program
 *
 * \param argc  number of arguments
 * \param argv  vector of arguments
 */
int	main(int argc, char *argv[]) {
	time_t	t;
	time(&t);
	int	c;
	int	longindex;
	Angle::unit	u = Angle::Degrees;
	while (EOF != (c = getopt_long(argc, argv, "dh?t:r", options,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 't':
			struct tm	tm;
			if (NULL == strptime(optarg, "%F %T", &tm)) {
				std::cerr << "cannot parse date: '" << optarg;
				std::cerr << "'" << std::endl;
				return EXIT_FAILURE;
			}
			t = mktime(&tm);
			break;
		case 'r':
			u = Angle::Revolutions;
			break;
		}
	}

	if (optind >= argc) {
		std::cerr << "no objects named" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	std::list<std::string>	bodies;
	for (int i = optind; i < argc; i++) {
		std::string	name(argv[i]);
		if (name == "all") {
			bodies.push_back(std::string("sun"));
			bodies.push_back(std::string("moon"));
		} else {
			bodies.push_back(name);
		}
	}

	solarsystem::SolarsystemBodyPtr	body;
	for (auto name : bodies) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "handle '%s'", name.c_str());
		body = solarsystem::SolarsystemFactory::get(name);
		if (!body) {
			std::cerr << name << " not found" << std::endl;
		} else {
			std::cout << body->ephemeris(t).toString();
			std::cout << " " << name << std::endl;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "loop");
	}
	
	return EXIT_SUCCESS;
}

} // namespace ephemeris
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::app::ephemeris::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by exception: " << x.what();
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
}

