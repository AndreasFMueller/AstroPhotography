/*
 * listcatalog.cpp -- list the stars in a catalog
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

using namespace astro::catalog;

namespace astro {
namespace app {
namespace listcatalog {

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "log",	no_argument,		NULL,		'l' }, /* 1 */
{ NULL,		0,			NULL,		 0  }, /* 2 */
};

void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "list the contents of a star catalog" << std::endl;
	std::cout << "usage:" << std::endl;
	std::cout << "   " << path.basename() << " [ options ] type filepath";
	std::cout << std::endl;
	std::cout << "<type> is one of BSC, Hipparcos, Tycho2, Ucac4, Combined,"
		" Database." << std::endl;
	std::cout << "Depending on <type>, the catalog at path <filepath> is "
		"openend and" << std::endl;
	std::cout << "the contents shown." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug    increase debug level" << std::endl;
	std::cout << " -l,--log      display error log" << std::endl;
	std::cout << std::endl;
}

CatalogFactory::BackendType	gettype(const std::string& type) {
	if (type == "BSC") {
		return CatalogFactory::BSC;
	}
	if (type == "Hipparcos") {
		return CatalogFactory::Hipparcos;
	}
	if (type == "Tycho2") {
		return CatalogFactory::Tycho2;
	}
	if (type == "Ucac4") {
		return CatalogFactory::Ucac4;
	}
	if (type == "Combined") {
		return CatalogFactory::Combined;
	}
	if (type == "Database") {
		return CatalogFactory::Database;
	}
	std::string	msg = stringprintf("'%s' is not a known backend type",
		type.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	showlog = false;
	while (EOF != (c = getopt_long(argc, argv, "dl", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'l':
			showlog = true;
			break;
		}

	// next two arguments are the type and the file name
	if (argc - optind < 2) {
		std::cerr << "missing arguments: type and path" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	type(argv[optind++]);
	std::string	file(argv[optind++]);

	CatalogFactory::BackendType	typecode = gettype(type);

	CatalogPtr	catalog = CatalogFactory::get(typecode, file);
	CatalogIterator	i;
	unsigned long long	counter = 0;
	unsigned long long	badcounter = 0;
	for (i = catalog->begin(); i != catalog->end(); ++i) {
		try {
			Star	s = *i;
			counter++;
			std::cout << counter << ": " << s.toString();
			std::cout << std::endl;
		} catch (const std::exception& x) {
			badcounter++;
			if (showlog) {
				std::cerr << "error at iterator position "
					<< counter << ": " << x.what()
					<< std::endl;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%llu stars listed, %llu rejected",
		counter, badcounter);
	return EXIT_SUCCESS;
}

} // namespace listcatalog
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::listcatalog::main>(argc, argv);
}
