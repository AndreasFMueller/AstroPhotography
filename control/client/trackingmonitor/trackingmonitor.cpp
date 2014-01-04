/*
 * trackingmonitor.cpp -- demo program for the tracking monitor functionality
 *                        of the astrod server
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <stdexception>
#include <cstdlib>

namespace astro {

int	trackingmonitor_main(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::trackingmonitor_main(argc, argv);
	} catch (std::exception& x) {
		std::cout << argv[0] << " terminated by exception: ";
		std::cout << x.what() << std::endl;
	}
}
