/**
 * stack.cpp -- test program for stacking
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroStacking.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroUtils.h>

using namespace astro;
using namespace astro::image;
using namespace astro::image::stacking;
using namespace astro::io;

namespace astro {
namespace app {
namespace stack {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "help",	no_argument,		NULL,		'h' }, /* 1 */
{ "output",	required_argument,	NULL,		'o' }, /* 2 */
{ NULL,		0,			NULL,		 0  }
};

void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -dh? ] [ -o outfile ] "
		"files..." << std::endl;
	std::cout << std::endl;
	std::cout << "stack a set of images to produce a target image. The "
		"file name arguments" << std::endl;
	std::cout << "are interpreted as FITS images to be stacked. "
		"All images are aligned with" << std::endl;
	std::cout << "the first image in the list and added to it. "
		"The resulting image is then" << std::endl;
	std::cout << "output to the output file." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d               increase debug level" << std::endl;
	std::cout << " -o outfile       filename of output file" << std::endl;
	std::cout << " -h,-?            display this help" << std::endl;
}

/**
 * \brief Main method for the stacker program
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	const char	*outfilename = NULL;
	while (EOF != (c = getopt_long(argc, argv, "dh?o:", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'o':
			outfilename = optarg;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	// read all the images
	ImageSequence	images;
	for (; optind < argc; optind++) {
		FITSin	in(argv[optind]);
		ImagePtr	image = in.read();
		images.push_back(image);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d images for sequence",
		images.size());

	// now do the stacking
	astro::image::stacking::Stacker	stacker;
	ImagePtr	stackedimage = stacker(images);

	// write the result image
	if(NULL != outfilename) {
		FITSout	out(outfilename);
		out.write(stackedimage);
	} else {
		std::cerr << "no output filename, not writing result image"
			 << std::endl;
	}

	// that's it
	return EXIT_SUCCESS;
}

} // namespace stack
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::stack::main>(argc, argv);
}
