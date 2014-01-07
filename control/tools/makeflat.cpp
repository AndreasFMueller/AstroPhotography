/*
 * makeflat.cpp -- create a flat frame from a set of FITS images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroIO.h>
#include <AstroFormat.h>

using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ -d?h ] [ -o outfile ] [ -D dark ] [ -o outfile ] files ..." << std::endl;
	std::cout << "compute flat frame from <files>..." << std::endl;
	std::cout << "options: " << std::endl;
	std::cout << " -d           increase debug level" << std::endl;
	std::cout << " -o outfile   write the computed flat file to <outfile>" << std::endl;
	std::cout << " -h, -?        show this help message" << std::endl;
	std::cout << " -D dark       use <dark> as the bias for flat computation" << std::endl;
}

/**
 * \brief Main function for makeflat tool 
 *
 * This tool takes a list of image names on the command line, reads them,
 * and produces a flat image from them.
 */
int	makeflat_main(int argc, char *argv[]) {
	char	*outfilename = NULL;
	const char	*darkfilename = NULL;
	int	c;
	while (EOF != (c = getopt(argc, argv, "do:D:?h")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'D':
			darkfilename = optarg;
			break;
		case 'o':
			outfilename = optarg;
			break;
		case '?':
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("bad option");
			break;
		}

	// make sure we do have some files to process
	if (argc <= optind) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no images specified");
		std::cerr << "no image file arguments specified" << std::endl;
	}

	// read the images into memory
	ImageSequence	images;
	for (; optind < argc; optind++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "reading file %s", argv[optind]);
		std::string	name(argv[optind]);
		FITSin	infile(name);
		ImagePtr	image = infile.read();
		images.push_back(image);
	}

	// Get the dark image. This can come from a file, in which case we
	// have to read the image from the file
	ImagePtr	dark;
	if (NULL != darkfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "reading dark image: %s",
			darkfilename);
		std::string	f = std::string(darkfilename);
		FITSin	infile(f);
		dark = infile.read();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got dark %d x %d",
			dark->size().width(), dark->size().height());
	} else {
		dark = ImagePtr(new Image<float>(images[0]->size()));
	}

	// now produce the flat image
	FlatFrameFactory	fff;
	ImagePtr	flat;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing flat image");
	flat = fff(images, dark);

	// display some info about the flat image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image %d x %d generated",
		flat->size().width(), flat->size().height());

	// write the flat image to a file
	if (outfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "outfile: %s", outfilename);
		unlink(outfilename);
		FITSout	outfile(outfilename);
		outfile.setPrecious(false);
		outfile.write(flat);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image written to %s",
			outfilename);
	}

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::makeflat_main(argc, argv);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "makeflat tool terminated by "
			"exception: %s", x.what());
		std::cerr << "makeflat tool terminated: " << x.what()
			<< std::endl;
	}
}
