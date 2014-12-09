/**
 * rescale.cpp -- take binary logarithm of all pixels of an image
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
#include <cmath>
#include <typeinfo>

using namespace astro;
using namespace astro::io;
using namespace astro::calibration;
using namespace astro::adapter;
using namespace astro::image::filter;

namespace astro {
namespace app {
namespace rescale {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "help",	no_argument,		NULL,		'h' }, /* 1 */
{ "min",	required_argument,	NULL,		'm' }, /* 2 */
{ "max",	required_argument,	NULL,		'M' }, /* 3 */
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief usage
 */
void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] infile outfile"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Stretch pixel values of an image. Source pixel values between" << std::endl;
	std::cout << "min and max are converted to values between 0 and 255." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "  -d,--debug      increase debug level" << std::endl;
	std::cout << "  -h,-?,--help    show this help message" << std::endl;
	std::cout << "  -m,--min=<min>  minimum source pixel value" << std::endl;
	std::cout << "  -M,--max=<max>  maximum max pixel value" << std::endl;
}

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	int	c;
	double	minimum = -1;
	double	maximum = -1;

	// parse the command line
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "d?hm:M:", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
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

	// create an adapter for the image
	ConstPixelValueAdapter<double>	from(image);

	// get minimum and maximum values from the input image
	if (maximum < 0) {
		maximum = Max<double, double>()(from);
	}
	if (minimum < 0) {
		minimum = Min<double, double>()(from);
	}
	double	scale = 255. / (maximum - minimum);

	// create image from rescaling adapter
	RescalingAdapter<double>	ra(from, minimum, scale);
	Image<double>	*rescaledimage = new Image<double>(ra);
	ImagePtr	outimage(rescaledimage);

	// after all the calibrations have been performed, write the output
	// file
	FITSout	outfile(outfilename);
	outfile.write(outimage);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace rescale
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::rescale::main>(argc, argv);
}
