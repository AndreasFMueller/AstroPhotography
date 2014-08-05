/*
 * starcatalog.cpp -- command line access to the star catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <stdlib.h>
#include <iostream>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroCatalog.h>

using namespace astro::catalog;

namespace astro {

RaDec	center(0, 0);
Angle	rawidth(1 * M_PI / 180);
Angle	decheight(1 * M_PI / 180);

/**
 * \brief Read a star by name from the database and show some info about it
 */
void	starmain(Catalog& catalog, const std::string& starname) {
	Star	star = catalog.find(starname);
	std::cout << star.toString() << std::endl;
}

void	areamain(Catalog& catalog, double minmag) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"list stars in area centered at %s/%s, w=%.3f, h=%.3f",
		center.ra().hms().c_str(), center.dec().dms().c_str(),
		rawidth.hours(), decheight.degrees());
	MagnitudeRange	magrange(-30, minmag);
	Catalog::starsetptr	stars = catalog.find(SkyWindow(center,
					rawidth, decheight), magrange);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stars returned: %d", stars->size());
	Catalog::starset::const_iterator	s;
	uint32_t	counter = 0;
	for (s = stars->begin(); s != stars->end(); s++) {
		if (magrange.contains(s->mag())) {
			std::cout << s->toString() << std::endl;
			counter++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u stars found", counter);
}

/**
 * \brief Main function for the starcatalog program
 */
int	main(int argc, char *argv[]) {
	int	c;
	std::string	path("/usr/local/starcatalogs");
	while (EOF != (c = getopt(argc, argv, "dp:m:R:D:H:W:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'p':
			path = std::string(optarg);
			break;
		case 'R':
			center.ra().hours(atof(optarg));
			break;
		case 'D':
			center.dec().degrees(atof(optarg));
			break;
		case 'H':
			decheight.degrees(atof(optarg));
			break;
		case 'W':
			rawidth.degrees(atof(optarg));
			break;
		}

	// open the star catalog
	Catalog	catalog(path);

	// next argument is what we want to see
	if (argc <= optind) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	command = std::string(argv[optind++]);
	if (command == "star") {
		for (; optind < argc; optind++) {
			starmain(catalog, std::string(argv[optind]));
		}
		return EXIT_SUCCESS;
	}
	if (command == "area") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "executing area command");
		double	minmag = 6.0;
		if (optind < argc) {
			minmag = atof(argv[optind++]);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"minimum magnitude: %.3f", minmag);
		}
		areamain(catalog, minmag);
		return EXIT_SUCCESS;
	}

	throw std::runtime_error("unknown command '" + command + "'");
}

} // namespace astro


int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "starcatalog terminated by exception: "
			<< x.what() << std::endl;
		return EXIT_FAILURE;
	}
}
