/**
 * gammacorrect.cpp -- gamma correct an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <iostream>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroIO.h>
#include <AstroDemosaic.h>
#include <AstroImager.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>
#include <AstroTonemapping.h>
#include <cmath>
#include <typeinfo>

using namespace astro;
using namespace astro::io;
using namespace astro::calibration;
using namespace astro::adapter;
using namespace astro::image::filter;

namespace astro {
namespace app {
namespace gammacorrect {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "gamma",	required_argument,	NULL,		'g' }, /* 2 */
{ "help",	no_argument,		NULL,		'h' }, /* 3 */
{ "min",	required_argument,	NULL,		'm' }, /* 4 */
{ "max",	required_argument,	NULL,		'M' }, /* 5 */
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief usage
 */
static void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ] infile outfile"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "  -d,--debug            increase debug level" << std::endl;
	std::cout << "  -f,--force            force image" << std::endl;
	std::cout << "  -g,--gamma=<gamma>    gamma value" << std::endl;
	std::cout << "  -m,--min=<min>        min value to correct" << std::endl;
	std::cout << "  -M,--max=<max>        max value to correct" << std::endl;
	std::cout << "  -h,-?,--help          show this help message" << std::endl;
}

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	int	c;
	double	gamma = 1.0;
	double	minimum = -1.;
	double	maximum = -1.;
	bool	force = false;

	// parse the command line
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "df?hm:M:g:", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
			break;
		case 'g':
			gamma = std::stod(optarg);
			break;
		case 'm':
			minimum = std::stod(optarg);
			break;
		case 'M':
			maximum = std::stod(optarg);
			break;
		case '?':
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// two more arguments are required: infile and outfile
	if (2 != argc - optind) {
		std::string	msg("wrong number of arguments");
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

	// convert pixels according to luminance
	ConstPixelValueAdapter<double>	from(image);

	// get the minimum and maximum values from the input image
	if (maximum < 0) {
		maximum = Max<double, double>()(from);
	}
	if (minimum < 0) {
		minimum = Min<double, double>()(from);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "min = %f, max = %f",
		minimum, maximum);

	// clamping filter
	ClampingAdapter<double, double>	ca(from, minimum, maximum);

	// rescaling
	double	scale = 1. / (maximum - minimum);
	RescalingAdapter<double>	ra(ca, minimum, scale);

	// gamma correction
	GammaAdapter<double>	ga(ra, gamma);

	// rescale back to the range 0-255
	RescalingAdapter<double>	ra2(ga, 0, 255.);

	// create image from last adapter
	Image<double>	*outimage = new Image<double>(ra2);
	ImagePtr	outimageptr(outimage);

	// remove previous file
	if (force) {
		unlink(outfilename.c_str());
	}

	// after all the calibrations have been performed, write the output
	// file
	FITSout	outfile(outfilename);
	outfile.write(outimageptr);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace gammacorrect
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::gammacorrect::main>(argc, argv);
}
