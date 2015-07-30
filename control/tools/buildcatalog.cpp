/*
 * buildcatalog.cpp -- utility to build a database catalog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include "../lib/catalogs/CatalogBackend.h"
#include <includes.h>
#include <iostream>
#include <typeinfo>
#include <AstroUtils.h>

using namespace astro::catalog;

namespace astro {
namespace app {
namespace buildcatalog {

static struct option	longopts[] = {
{ "bsc",	required_argument,	NULL,		'B' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	required_argument,	NULL,		'h' }, /* 2 */
{ "hipparcos",	required_argument,	NULL,		'H' }, /* 3 */
{ "tycho2",	required_argument,	NULL,		'T' }, /* 4 */
{ "ucac4",	required_argument,	NULL,		'U' }, /* 5 */
{ NULL,		0,			NULL,		0   }
};

void	usage(const char *progname) {
	std::cout << "usage: " << std::endl;
	std::cout << "    " << progname << " [ options ] dbfile" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " --debug,-d            increase debug level" << std::endl;
	std::cout << " --help,-h,-?          display this help message";
	std::cout << std::endl;
	std::cout << " -B,--bsc=dir          Bright Star Catalog directory";
	std::cout << std::endl;
	std::cout << " -H,--hipparcos=dir    Hipparcos catalog directory";
	std::cout << std::endl;
	std::cout << " -T,--tycho2=dir       Tycho2 catalog directory";
	std::cout << std::endl;
	std::cout << " -U,--ucac4=dir        Ucacu4 catalog directory";
	std::cout << std::endl;
}

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	std::string	bscfile;
	std::string	hipparcosfile;
	std::string	tycho2file;
	std::string	ucac4dir;
	while (EOF != (c = getopt_long(argc, argv, "B:dhH:T:U:?", longopts,
		&longindex)))
		switch (c) {
		case 'B':
			bscfile = std::string(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'H':
			hipparcosfile = std::string(optarg);
			break;
		case 'T':
			tycho2file = std::string(optarg);
			break;
		case 'U':
			ucac4dir = std::string(optarg);
			break;
		}

	// the remaining argument ist the databasename
	if (optind >= argc) {
		throw std::runtime_error("database filename argument missing");
	}
	std::string	databasefilename(argv[optind++]);

	// open the database catalog
	DatabaseBackendCreator	database(databasefilename);
	database.prepare();

	// open the Bright star catalog
	if (bscfile.size()) {
		CatalogPtr	catalog
			= CatalogFactory::get(CatalogFactory::BSC, bscfile);
		CatalogIterator	i;
		for (i = catalog->begin(); i != catalog->end(); ++i) {
			Star	s = *i;
			database.add(s);
		}
	}

	// open the hipparcos catalog
	if (hipparcosfile.size()) {
		int	counter = 0;
		CatalogPtr	catalog
			= CatalogFactory::get(CatalogFactory::Hipparcos,
				hipparcosfile);
		CatalogIterator	i;
		for (i = catalog->begin(); i != catalog->end(); ++i) {
			Star	s = *i;
			database.add(s);
			counter++;
			if (0 == (counter % 10000)) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%d Hipparcos stars added", counter);
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "total %d Hipparcos stars added",
			counter);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Hipparcos catalog disabled");
	}

	// open the Tycho2 catalog
	if (tycho2file.size()) {
		int	counter = 0;
		CatalogPtr	catalog
			= CatalogFactory::get(CatalogFactory::Tycho2,
				tycho2file);
		CatalogIterator	i;
		for (i = catalog->begin(); i != catalog->end(); ++i) {
			try {
				Star	s = *i;
				database.add(s);
				counter++;
				if (0 == (counter % 100000)) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"%d Tycho2 stars added",
						counter);
				}
			} catch (const std::exception x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"tycho2 catalog error at %d: %s",
					counter, x.what());
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "total %d Tycho2 stars added",
			counter);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Tycho2 catalog disabled");
	}

	// open the Ucac4 catalog
	if (ucac4dir.size()) {
		int	counter = 0;
		CatalogPtr	catalog
			= CatalogFactory::get(CatalogFactory::Ucac4,
				ucac4dir);
		CatalogIterator	i;
		for (i = catalog->begin(); i != catalog->end(); ++i) {
			Star	s = *i;
			database.add(s);
			counter++;
			if (0 == (counter % 1000000)) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%d Ucac4 stars added", counter);
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "total %d Ucac4 stars added",
			counter);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "UCAC4 catalog disabled");
	}

	// cleanup
	database.finalize();

	// create an index for RA/DEC
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "creating index");
		database.createindex();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "index created");
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
