/*
 * astromount.cpp -- get or set position on a mount
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <cstdlib>

namespace astro {

int mount_main(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::mount_main>(argc, argv);
}
