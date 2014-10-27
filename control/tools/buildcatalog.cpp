/*
 * buildcatalog.cpp -- utility to build a database catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <CatalogBackend.h>
#include <Hipparcos.h>
#include <Tycho2.h>
#include <Ucac4.h>
#include <includes.h>
#include <iostream>
#include <typeinfo>
#include <AstroUtils.h>

using namespace astro::catalog;

namespace astro {
namespace app {
namespace buildcatalog {

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "hipparchos", required_argument,	NULL,		'h' }, /* 1 */
{ "tycho2",	required_argument,	NULL,		't' }, /* 2 */
{ "ucac4",	required_argument,	NULL,		'u' }, /* 3 */
{ NULL,		0,			NULL,		0   }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	std::string	hipfile;
	std::string	tycho2file;
	std::string	ucac4dir;
	while (EOF != (c = getopt_long(argc, argv, "dh:t:u:", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			hipfile = std::string(optarg);
			break;
		case 't':
			tycho2file = std::string(optarg);
			break;
		case 'u':
			ucac4dir = std::string(optarg);
			break;
		}

	// the remaining argument ist the databasename
	if (optind >= argc) {
		throw std::runtime_error("database filename argument missing");
	}
	std::string	databasefilename(argv[optind++]);

	// open the database catalog
	DatabaseBackend	database(databasefilename);
	database.clear();
	database.prepare();
	int	id = 0;
	int	counter = 0;

	// open the hipparcos catalog
#if 0
	Hipparcos	hipparcos(hipfile);
	Hipparcos::const_iterator	s;
	for (s = hipparcos.begin(); s != hipparcos.end(); s++) {
		Star	star = s->second;
		database.add(id++, star);
		counter++;
		if (0 == (counter % 10000)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars added from "
				"Hipparcos so far", counter);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars added from Hipparcos", counter);
#else
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Hipparcos catalog disabled");
#endif

	// open the Tycho2 catalog
#if 0
	Tycho2	tycho2(tycho2file);
	unsigned int	index;
	counter = 0;
	for (index = 0; index < tycho2.nstars(); index++) {
		try {
			Tycho2Star	tycho2star = tycho2.find(index);
			if (!tycho2star.isHipparcosStar()) {
				Star	star = tycho2star;
				std::string	name = stringprintf("T%u",
							index);
				database.add(id++, star);
				counter++;
				if (0 == (counter % 10000)) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"%d stars added from "
						"Tycho2 so far", counter);
				}
			}
		} catch (const std::exception& x) {
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping index %u: %s",
			//	index, x.what());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars added from Tycho2", counter);
#else
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tycho-2 catalog disabled");
#endif

#if 1
	// open UCAC4 catalog
	Ucac4	ucac4(ucac4dir);
	counter = 0;
	for (uint16_t zonenumber = 1; zonenumber <= 900; zonenumber++) {
		Ucac4ZonePtr	zone = ucac4.zone(zonenumber);
		for (uint32_t number = 1; number <= zone->nstars(); number++) {
			Ucac4Star	ucac4star = zone->get(number);
			if (!ucac4star.hiptyc2) {
				Star	star = ucac4star;
				database.add(id++, star);
				counter++;
				if (0 == (counter % 10000)) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"%d stars added from Ucac4 so far",
						counter);
				}
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars added from Ucac4", counter);
#else
	debug(LOG_DEBUG, DEBUG_LOG, 0, "UCAC4 catalog disabled");
#endif

	database.finalize();

	try {
		database.createindex();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error while creating index: %s",
			x.what());
	}

	return EXIT_SUCCESS;
}

} // namespace buildcatalog
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::buildcatalog::main>(argc, argv);
}
