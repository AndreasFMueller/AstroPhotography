/*
 * makedark.cpp -- create a dark frame from a set of FITS images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroIO.h>

using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ] darkimages"
		<< std::endl;
	std::cout <<"Computes a consolidated dark image from a set of images"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d             increase debug level" << std::endl;
	std::cout << "  -h, -?         show this help message" << std::endl;
	std::cout << "  -o outfile     filename of the output dark image"
		<< std::endl;
}

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a list of image names on the command line, reads them,
 * and produces a dark image from them.
 */
int	main(int argc, char *argv[]) {
	char	*outfilename = NULL;
	int	c;
	bool	gridded = false;
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
			break;
		}

	if (argc <= optind) {
		std::string	msg("no images specified");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		std::cerr << "no image file arguments specified" << std::endl;
		throw std::runtime_error(msg);
	}

	ImageSequence	images;
	for (; optind < argc; optind++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "reading file %s", argv[optind]);
		std::string	name(argv[optind]);
		FITSin	infile(name);
		ImagePtr	image = infile.read();
		images.push_back(image);
	}

	DarkFrameFactory	dff;
	ImagePtr	dark = dff(images);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image %d x %d generated",
		dark->size.width, dark->size.height);

	if (outfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "outfile: %s", outfilename);
		unlink(outfilename);
		FITSout	outfile(outfilename);
		outfile.write(dark);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image written to %s",
			outfilename);
	}

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "makedark tool terminated by "
			"exception: %s", x.what());
		std::cerr << "makedark tool terminated: " << x.what()
			<< std::endl;
	}
}
