/*
 * buildcatalog.cpp -- utility to build a database catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <DatabaseCatalog.h>
#include <Hipparcos.h>
#include <Tycho2.h>
#include <Ucac4.h>
#include <includes.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {

int	main(int argc, char *argv[]) {
	int	c;
	std::string	hipfile;
	std::string	tycho2file;
	std::string	ucac4dir;
	while (EOF != (c = getopt(argc, argv, "dh:t:u:")))
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
	DatabaseCatalog	database(databasefilename);
	database.clear();
	int	id = 0;
	int	counter = 0;

	// open the hipparcos catalog
	Hipparcos	hipparcos(hipfile);
#if 1
	Hipparcos::const_iterator	s;
	for (s = hipparcos.begin(); s != hipparcos.end(); s++) {
		Star	star = s->second;
		database.add(id++, star, stringprintf("HIP%u", s->second.hip));
		counter++;
		if (0 == (counter % 10000)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars added from "
				"Hipparcos so far", counter);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars added from Hipparcos", counter);
#endif

	// open the Tycho2 catalog
	Tycho2	tycho2(tycho2file);
#if 1
	unsigned int	index;
	counter = 0;
	for (index = 0; index < tycho2.nstars(); index++) {
		try {
			Tycho2Star	tycho2star = tycho2.find(index);
			if (!tycho2star.isHipparcosStar()) {
				Star	star = tycho2star;
				std::string	name = stringprintf("T%u",
							index);
				database.add(id++, star, name);
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
#endif

	// open UCAC4 catalog
	Ucac4	ucac4(ucac4dir);
	counter = 0;
	for (uint16_t zonenumber = 1; zonenumber <= 900; zonenumber++) {
		Ucac4ZonePtr	zone = ucac4.zone(zonenumber);
		for (uint32_t number = 1; number <= zone->nstars(); number++) {
			Ucac4Star	ucac4star = zone->get(number);
			Star	star = ucac4star;
			database.add(id++, star, ucac4star.number.toString());
			counter++;
			if (0 == (counter % 10000)) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%d stars added from Ucac4 so far",
					counter);
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars added from Ucac4", counter);

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by exception: " << x.what()
			<< std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}