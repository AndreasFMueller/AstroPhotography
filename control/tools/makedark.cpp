/*
 * makedark.cpp -- create a dark frame from a set of FITS images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <debug.h>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroIO.h>

using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;

namespace astro {

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a list of image names on the command line, reads them,
 * and produces a dark image from them.
 */
int	main(int argc, char *argv[]) {
	char	*outfilename = NULL;
	int	c;
	while (EOF != (c = getopt(argc, argv, "do:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'o':
			outfilename = optarg;
			break;
		}

	if (argc <= optind) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no images specified");
		std::cerr << "no image file arguments specified" << std::endl;
	}

	ImageSequence	images;
	for (; optind < argc; optind++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "reading file %s", argv[optind]);
		FITSin	infile(std::string(argv[optind]));
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
