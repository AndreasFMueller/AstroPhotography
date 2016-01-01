/**
 * limit.cpp -- limit pixel values into an interval
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdlib.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <stacktrace.h>
#include <typeinfo>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace app {
namespace limit {

int	main(int argc, char *argv[]) {
	int	c;
	double	infimum = 0;
	double	supremum = 0;

	while (EOF != (c = getopt(argc, argv, "i:s:d")))
		switch (c) {
		case 'i':
			infimum = atof(optarg);
			break;
		case 's':
			supremum = atof(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
		}

	// next two arguments must be file names
	if ((argc - optind) != 2) {
		std::cerr << "need exactly two file name arguments" << std::endl;
		return EXIT_FAILURE;
	}
	char	*infilename = optarg
}

} // namespace limit
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::limit::main>(argc, argv);
}
