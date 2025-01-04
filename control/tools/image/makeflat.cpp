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
#include <AstroUtils.h>

using namespace astro;
using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;

namespace astro {
namespace app {
namespace makeflat {

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] files ..."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "compute flat frame from <files>..." << std::endl;
	std::cout << std::endl;
	std::cout << "options: " << std::endl;
	std::cout << std::endl;
	std::cout << " -d,--debug               increase debug level"
		<< std::endl;
	std::cout << " -o,--outfile=<outfile>   write the computed flat file "
		"to <outfile>" << std::endl;
	std::cout << " -h,-?,--help             show this help message"
		<< std::endl;
	std::cout << " -D,--dark=<dark>         use <dark> as the bias for "
		"flat computation" << std::endl;
	std::cout << " -m,--mosaic              normalize each channel of an Bayer mosaic individually" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,	'd' }, /* 0 */
{ "outfile",	required_argument,	NULL,	'o' }, /* 0 */
{ "help",	no_argument,		NULL,	'h' }, /* 0 */
{ "dark",	required_argument,	NULL,	'D' }, /* 0 */
{ "mosaic",	required_argument,	NULL,	'm' }, /* 0 */
{ NULL,		0,			NULL,	 0  }, /* 0 */
};

/**
 * \brief Main function for makeflat tool 
 *
 * This tool takes a list of image names on the command line, reads them,
 * and produces a flat image from them.
 */
int	main(int argc, char *argv[]) {
	char	*outfilename = NULL;
	const char	*darkfilename = NULL;
	int	c;
	int	longindex;
	bool	mosaic = false;
	while (EOF != (c = getopt_long(argc, argv, "do:D:?hm",
		longopts, &longindex)))
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
		case 'm':
			mosaic = true;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "computing flat image%s",
		(mosaic) ? " (mosaic)" : "");
	flat = fff(images, dark, mosaic);

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

} // namespace makeflat
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::makeflat::main>(argc, argv);
}
