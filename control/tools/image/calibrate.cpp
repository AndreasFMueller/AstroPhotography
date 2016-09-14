/*
 * calibrate images using darks and flats
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <iostream>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroInterpolation.h>
#include <AstroIO.h>
#include <AstroDemosaic.h>
#include <AstroImager.h>
#include <AstroUtils.h>
#include <AstroOperators.h>
#include <AstroAdapter.h>

using namespace astro;
using namespace astro::io;
using namespace astro::calibration;
using namespace astro::interpolation;
using namespace astro::camera;

namespace astro {
namespace app {
namespace calibrate {

/**
 * \brief usage function to display command help
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] infile outfile"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "  -D,--dark=<dark.fits>   use image file <dark> for dark correction"
		<< std::endl;
	std::cout << "  -F,--flat=<flat.fits>   use image file <flat> for flat correction"
		<< std::endl;
	std::cout << "  -m,--min=<min>          clamp the image values to at least <min>"
		<< std::endl;
	std::cout << "  -M,--max=<max>          clamp the image values to at most <max>"
		<< std::endl;
	std::cout << "  -b,--bayer              demosaic bayer images" << std::endl;
	std::cout << "  -f,--flip               flip image (useful for HyperStar)" << std::endl;
	std::cout << "  -i,--interpolate        interpolate bad pixels" << std::endl;
	std::cout << "  -d,--debug              increase debug level" << std::endl;
	std::cout << "  -h,-?,--help            show this help message" << std::endl;
}

static struct option	longopts[] = {
{ "bayer",		no_argument,		NULL,	'b' }, /* 0 */
{ "debug",		no_argument,		NULL,	'd' }, /* 1 */
{ "dark",		required_argument,	NULL,	'D' }, /* 2 */
{ "flat",		required_argument,	NULL,	'F' }, /* 3 */
{ "flip",		no_argument,		NULL,	'f' }, /* 4 */
{ "help",		no_argument,		NULL,	'h' }, /* 5 */
{ "min",		required_argument,	NULL,	'm' }, /* 6 */
{ "max",		required_argument,	NULL,	'M' }, /* 7 */
{ "interpolate",	no_argument,		NULL,	'i' }, /* 8 */
{ NULL,			0,			NULL,	 0  }, /* 9 */
};

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	const char	*darkfilename = NULL;
	const char	*flatfilename = NULL;
	double	minvalue = -1;
	double	maxvalue = -1;
	bool	demosaic = false;
	bool	interpolate = false;
	bool	flip = false;

	// parse the command line
	while (EOF != (c = getopt_long(argc, argv, "dD:F:?hfm:M:bi",
		longopts, &longindex)))
		switch (c) {
		case 'b':
			demosaic = true;
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'D':
			darkfilename = optarg;
			break;
		case 'F':
			flatfilename = optarg;
			break;
		case 'f':
			flip = true;
			break;
		case '?':
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			interpolate = true;
			break;
		case 'm':
			minvalue = atof(optarg);
			break;
		case 'M':
			maxvalue = atof(optarg);
			break;
		}

	// two more arguments are required: infile and outfile
	if (2 != argc - optind) {
		std::string	msg = stringprintf("wrong number of arguments: "
			"%d instead of 2", argc - optind);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::string	infilename(argv[optind++]);
	std::string	outfilename(argv[optind]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrate %s to %s",
		infilename.c_str(), outfilename.c_str());

	// read the infile
	FITSin	infile(infilename);
	ImagePtr	image = infile.read();

	// build the Imager
	Imager	imager;

	// if we have a dark correction, apply it
	ImagePtr	dark;
	if (NULL != darkfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark correct: %s",
			darkfilename);
		FITSin	darkin(darkfilename);
		dark = darkin.read();
		imager.dark(dark);
		imager.darksubtract(true);
	}

	// if we have a flat file, we perform flat correction
	if (NULL != flatfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flat correction: %s",
			flatfilename);
		FITSin	flatin(flatfilename);
		ImagePtr	flat = flatin.read();
		imager.flat(flat);
		imager.flatdivide(true);
	}

	// perform bad pixel interpolation
	if (interpolate) {
		imager.interpolate(true);
	}

	// apply imager corrections
	imager(image);

	// if minvalue or maxvalue are set, clamp the image values
	if ((minvalue >= 0) || (maxvalue >= 0)) {
		if (minvalue < 0) {
			minvalue = 0;
		}
		if (maxvalue < 0) {
			maxvalue = std::numeric_limits<double>::infinity();
		}
		Clamper	clamp(minvalue, maxvalue);
		clamp(image);
	}

	// if demosaic is requested we do that now
	ImagePtr	outimage;
	if (demosaic) {
		outimage = demosaic_bilinear(image);
		if (image->hasMetadata("PROJECT")) {
			outimage->setMetadata(image->getMetadata("PROJECT"));
		}
		if (image->hasMetadata("INSTRUME")) {
			outimage->setMetadata(image->getMetadata("INSTRUME"));
		}
		//astro::image::operators::colorscaling_operator(RGB<double>(0.57,0.88,1), outimage);
	} else {
		outimage = image;
	}

	// flip the image
	if (flip) {
		astro::image::operators::flip(outimage);
	}

	// after all the calibrations have been performed, write the output
	// file
	FITSout	outfile(outfilename);

	// set attributes that we inherit from the original image
	outfile.write(outimage);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace calibrate
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::calibrate::main>(argc, argv);
}
