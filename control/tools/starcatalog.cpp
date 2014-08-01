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

Ucac4Ptr	ucac4;
RaDec	center(0, 0);
Angle	rawidth(1 * M_PI / 180);
Angle	decheight(1 * M_PI / 180);

/**
 * \brief Read a star by name from the database and show some info about it
 */
void	starmain(const std::string& starname) {
	Ucac4StarNumber	number(starname);
	Ucac4Star	star = ucac4->find(number);
	std::cout << star.number 
		<< ": RA=" << star.position.ra().hms(' ')
		<< ", DEC=" << star.position.dec().dms(' ')
		<< ", mag=" << star.mag1
		<< ", Jmag=" << star.mag_j
		<< std::endl;
}

void	zonemain(const uint16_t z, double minmag) {
	std::cout << "list zone " << z << ", minimum magnitude " << minmag
		<< std::endl;
	Ucac4ZonePtr	zone = ucac4->zone(z);
	uint32_t	counter = 0;
	for (uint32_t n = 1; n <= zone->nstars(); n++) {
		Ucac4Star	star = zone->get(n);
		if (star.mag1 <= minmag) {
			std::cout << star.toString() << std::endl;
			counter++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%u stars brighter than %.3f in zone %hu",
		counter, minmag, z);
/*
	uint32_t	middle = zone->first(Angle(M_PI));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "middle of zone: %u", middle);
*/
}

void	areamain(double minmag) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"list stars in area centered at %s/%s, w=%.3f, h=%.3f",
		center.ra().hms().c_str(), center.dec().dms().c_str(),
		rawidth.hours(), decheight.degrees());
	std::set<Ucac4Star>	stars = ucac4->find(SkyWindow(center,
					rawidth, decheight));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stars returned: %d", stars.size());
	std::set<Ucac4Star>::const_iterator	s;
	uint32_t	counter = 0;
	for (s = stars.begin(); s != stars.end(); s++) {
		if (minmag > s->mag1) {
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
	std::string	path("/usr/local/u4");
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
	ucac4 = Ucac4Ptr(new Ucac4(path));

	// next argument is what we want to see
	if (argc <= optind) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	command = std::string(argv[optind++]);
	if (command == "star") {
		for (; optind < argc; optind++) {
			starmain(std::string(argv[optind]));
		}
		return EXIT_SUCCESS;
	}
	if (command == "zone") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "executing zone command");
		if (optind >= argc) {
			throw std::runtime_error("not enough arguments");
		}
		uint16_t	zone = atoi(argv[optind++]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "zone = %hu", zone);
		double	minmag = 6.0;
		if (optind < argc) {
			minmag = atof(argv[optind++]);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"minimum magnitude: %.3f", minmag);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for stars with "
			"mag >= %.3f", minmag);
		zonemain(zone, minmag);
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
		areamain(minmag);
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
