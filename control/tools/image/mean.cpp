/*
 * mean.cpp -- compute the mean of the images
 *
 * (c) 2026 Prof Dr Andreas Müller
 */
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <AstroDebug.h>

namespace astro {
namespace app {
namespace mean {

/**
 * \brief Write a help message
 *
 * \param progname 	name under which the program was called
 */
static void	usage(const char *progname) {
	std::cout << "usage:" << std::endl;
	std::cout << "   " << progname << " [ options ] files" << std::endl;
	std::cout << "options:" << std::endl;
}

/**
 * \brief Options for the fold program
 */
static struct option    longopts[] = {
{ "debug",      no_argument,            NULL,           'd' },
{ "help",	no_argument,		NULL,		'h' },
{ "outfile",	required_argument,	NULL,		'o' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief Main function for the mean program
 *
 * \param argc	number of arguments
 * \param argv	parameter array
 */
static int	main(int argc, char *argv[]) {
	std::string	outfilename;

	// parse command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?o",
			longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'o':
			outfilename = std::string(optarg);
			break;
		}

	// read all the input files
	while (optind < argc) {
		std::string	infilename(argv[optind++]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing file %s",
			infilename.c_str());
	}

	// write output file

	return EXIT_SUCCESS;
}

} // namespace mean
} // namespace app
} // app

int	main(int argc, char *argv[]) {
	try {
		return astro::app::mean::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "mean programm terminated by exception "
			<< x.what() << std::endl;
	} catch (...) {
		std::cerr << "mean programm terminated by unknown exception"
			<< std::endl;
	}
	return EXIT_FAILURE;
}
