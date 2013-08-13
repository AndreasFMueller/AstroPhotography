/**
 * stack.cpp -- test program for stacking
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroStacking.h>
#include <AstroDebug.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::image::stacking;
using namespace astro::io;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ -dh? ] [ -o outfile ] files..." << std::endl;
	std::cout << "stack a set of images to produce a target image" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d               increase debug level" << std::endl;
	std::cout << " -o outfile       filename of output file" << std::endl;
	std::cout << " -h,-?            display this help" << std::endl;
}

int	main(int argc, char *argv[]) {
	int	c;
	const char	*outfilename = NULL;
	while (EOF != (c = getopt(argc, argv, "do:")))
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

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "stack terminated by exception: " << x.what()
			<< std::endl;
	}
}