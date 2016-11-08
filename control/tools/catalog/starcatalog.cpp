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
#include <AstroUtils.h>

using namespace astro;
using namespace astro::catalog;

namespace astro {
namespace app {
namespace starcatalog {

RaDec	center(0, 0);
Angle	rawidth(1 * M_PI / 180);
Angle	decheight(1 * M_PI / 180);

/**
 * \brief Read a star by name from the database and show some info about it
 */
void	starmain(CatalogPtr catalog, const std::string& starname) {
	Star	star = catalog->find(starname);
	std::cout << star.toString() << std::endl;
}

void	areamain(CatalogPtr catalog, double minmag) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"list stars in area centered at %s/%s, w=%.3f, h=%.3f",
		center.ra().hms().c_str(), center.dec().dms().c_str(),
		rawidth.hours(), decheight.degrees());
	MagnitudeRange	magrange(-30, minmag);
	Catalog::starsetptr	stars = catalog->find(SkyWindow(center,
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

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] star <starname>"
		<< std::endl;
	std::cout << "    " << p.basename() << " [ options ] area <minag>"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "extract a star by name from the catalog (first syntax) "
		"or extract stars" << std::endl;
	std::cout << "in an area determined by the RA and DEC options "
		"(second syntax). In the " << std::endl;
	std::cout << "latter case, only display stars brighter than <minmag>."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug             increase debug level"
		<< std::endl;
	std::cout << "    -p,--path=<path>       path to the star catalogs"
		<< std::endl;
	std::cout << "    -R,--ra=<ra>           right ascension of center"
		<< std::endl;
	std::cout << "    -D,--dec=<dec>         declination of center"
		<< std::endl;
	std::cout << "    -H,--height=<height>   angular height of image"
		<< std::endl;
	std::cout << "    -W,--width=<width>     angular width of image"
		<< std::endl;
	std::cout << "    -h,-?,--help           display this help emssage"
		<< std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,	'd' }, /* 0 */
{ "path",	required_argument,	NULL,	'p' }, /* 1 */
{ "ra",		required_argument,	NULL,	'R' }, /* 2 */
{ "dec",	required_argument,	NULL,	'D' }, /* 3 */
{ "height",	required_argument,	NULL,	'H' }, /* 4 */
{ "width",	required_argument,	NULL,	'W' }, /* 5 */
{ "help",	required_argument,	NULL,	'h' }, /* 6 */
{ NULL,		0,			NULL,	 0  }, /* 7 */
};

/**
 * \brief Main function for the starcatalog program
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	std::string	path("/usr/local/starcatalogs");
	while (EOF != (c = getopt_long(argc, argv, "dp:R:D:H:W:h?",
		longopts, &longindex)))
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
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}

	// open the star catalog
	CatalogPtr	catalog = CatalogFactory::get(CatalogFactory::Combined,
					path);

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

} // namespace starcatalog
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::starcatalog::main>(argc, argv);
}
