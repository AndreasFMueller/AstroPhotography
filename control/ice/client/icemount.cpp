/*
 * icemount.cpp -- query or position mount
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <cstdlib>

namespace snowstar {

int	icemount_main(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}

} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::icemount_main>(argc, argv);
}
