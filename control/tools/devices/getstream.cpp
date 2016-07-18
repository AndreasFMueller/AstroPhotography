/*
 * getstream.cpp -- get a stream of images from a camera
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroDebug.h>

namespace astro {
namespace app {
namespace getstream {

static void	usage(const char *progname) {
}

int	main(int argc, char *argv[]) {
	usage(argv[0]);
	return EXIT_SUCCESS;
}

} // namespace getstream
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::getstream::main>(argc, argv);
}
