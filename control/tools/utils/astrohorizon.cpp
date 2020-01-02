/*
 * astrohorizon.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroHorizon.h>
#include <includes.h>

using namespace astro::horizon;

namespace astro {
namespace app {
namespace horizon {

/**
 * \brief Table of options for the astrofocus program
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief Help function
 */
static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] <filename>" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug           enter debug mode"
		<< std::endl;
	std::cout << " -h,--help            display this help message and exit"
		<< std::endl;
}

/**
 * \brief Main function for the astrofocus program
 *
 * \param argc	number of arguments
 * \param argv	vector of arguments
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astrohorizon");
	debugthreads = 1;
	int	c;
	int	longindex;
	putenv((char *)"POSIXLY_CORRECT=1");    // cast to silence compiler
	while (EOF != (c = getopt_long(argc, argv, "dh?",
		longopts, &longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	// if three are no further arguments, send an error message
	HorizonPtr	horizon;
	if (optind >= argc) {
		horizon = Horizon::get();
	} else {
		std::string	filename(argv[optind++]);
		horizon = HorizonPtr(new Horizon(filename));
	}

	// display all the points
	int	counter = 0;
	Horizon::const_iterator	i;
	for (i = horizon->begin(), counter = 0; i != horizon->end();
		i++, counter++) {
		std::cout << stringprintf("[%3d]  ", counter);
		std::cout << i->azm().dms();
		std::cout << " ";
		std::cout << i->alt().dms();
		std::cout << std::endl;
	}
	
	// done
	return EXIT_SUCCESS;
}

} // namespace horizon
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main_function<astro::app::horizon::main>(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "cannot process horizon file: " << x.what();
		std::cerr << std::endl;
	}
}


