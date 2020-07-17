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
{ "perturbation",	no_argument,			NULL,		'p' },
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
	bool	use_perturbation = false;
	while (EOF != (c = getopt_long(argc, argv, "dh?t:rp", options,
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
		case 'p':
			use_perturbation = true;
			break;
		}
	}

	if (optind >= argc) {
		std::cerr << "no objects named" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	// get the current time in julian centuries
	solarsystem::JulianCenturies	T(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time: %f", (double)T);

	// get the position of the earth
	solarsystem::EclipticalCoordinates	earth_pos;
	if (use_perturbation) {
		solarsystem::EarthPerturbed	earth_perturbed;
		earth_pos = earth_perturbed.ecliptical(T);
	} else {
		solarsystem::Earth		earth;
		earth_pos = earth.ecliptical(T);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "earth position  : %s",
		earth_pos.toString().c_str());

	// compute the RaDec for the sun
	solarsystem::RelativePosition	rp(T, earth_pos);
	RaDec	radec = rp.radec(Vector(0,0,0));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sun position: %s",
		radec.toString().c_str());

	// compute the position of the planets
	for (int i = optind; i < argc; i++) {
		RaDec	radec;
		solarsystem::Planetoid	*planet = NULL;
		std::string	name(argv[i]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "handle '%s'", name.c_str());
		if ((name == "mercury") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::MercuryPerturbed();
			} else {
				planet = new solarsystem::Mercury();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
		if ((name == "venus") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::VenusPerturbed();
			} else {
				planet = new solarsystem::Venus();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
		if ((name == "mars") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::MarsPerturbed();
			} else {
				planet = new solarsystem::Mars();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
		if ((name == "jupiter") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::JupiterPerturbed();
			} else {
				planet = new solarsystem::Jupiter();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
		if ((name == "saturn") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::SaturnPerturbed();
			} else {
				planet = new solarsystem::Saturn();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
		if ((name == "uranus") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::UranusPerturbed();
			} else {
				planet = new solarsystem::Uranus();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
		if ((name == "neptune") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::NeptunePerturbed();
			} else {
				planet = new solarsystem::Neptune();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
		if ((name == "pluto") || (name == "all")) {
			if (use_perturbation) {
				planet = new solarsystem::PlutoPerturbed();
			} else {
				planet = new solarsystem::Pluto();
			}
			radec = rp.radec(planet);
			std::cout << radec.toString();
			std::cout << " " << planet->name() << std::endl;
			delete planet;
		}
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

