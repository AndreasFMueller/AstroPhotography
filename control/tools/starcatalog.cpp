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

/**
 * \brief Read a star by name from the database and show some info about it
 */
void	starmain(const std::string& starname) {
	Ucac4Star	star = ucac4->find(starname);
	std::cout << starname
		<< ": RA=" << star.position.ra().hms(' ')
		<< ", DEC=" << star.position.dec().dms(' ')
		<< ", mag=" << star.mag1
		<< ", Jmag=" << star.mag_j
		<< std::endl;
}

/**
 * \brief Main function for the starcatalog program
 */
int	main(int argc, char *argv[]) {
	int	c;
	std::string	path("/usr/local/u4");
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'p':
			path = std::string(optarg);
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
