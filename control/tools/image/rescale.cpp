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
{ "scale",	required_argument,	NULL,		's' }, /* 4 */
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief usage
 */
static void	usage(const char *progname) {
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
	std::cout << "  -d,--debug          increase debug level" << std::endl;
	std::cout << "  -h,-?,--help        show this help message" << std::endl;
	std::cout << "  -m,--min=<min>      minimum source pixel value" << std::endl;
	std::cout << "  -M,--max=<max>      maximum max pixel value" << std::endl;
	std::cout << "  -s,--scale=<scale>  scale factor for pixel values" << std::endl;
	std::cout << "Note that the -s option can not be given" << std::endl;
	std::cout << "simultanously with the -M option" << std::endl;
}

double	minimum = -1;
double	maximum = -1;
double	scale = -1;

template<typename Pixel>
ImagePtr	rescale(const ConstImageAdapter<Pixel>& image) {
	LuminanceAdapter<Pixel, double>	luminance(image);
	if (minimum < 0) {
		minimum = Min<double, double>()(luminance);
	}
	if (scale < 0) {
		if (maximum < 0) {
			maximum = Max<double, double>()(luminance);
		}
		scale = 255. / (maximum - minimum);
	}

	// create image from rescaling adapter
	Pixel	zero(minimum);
	RescalingAdapter<Pixel>	ra(image, zero, scale);
	Image<Pixel>	*rescaledimage = new Image<Pixel>(ra);
	ImagePtr	outimage(rescaledimage);
	return outimage;
}

#define	do_rescale(image, Pixel)			\
{							\
	Image<Pixel >	*imagep				\
		= dynamic_cast<Image<Pixel >*>(&*image);\
	if (NULL != imagep) {				\
		return rescale(*imagep);		\
	}						\
}

ImagePtr	rescale(ImagePtr image) {
	do_rescale(image, unsigned char)
	do_rescale(image, unsigned short)
	do_rescale(image, unsigned int)
	do_rescale(image, unsigned long)
	do_rescale(image, float)
	do_rescale(image, double)
	do_rescale(image, RGB<unsigned char>)
	do_rescale(image, RGB<unsigned short>)
	do_rescale(image, RGB<unsigned int>)
	do_rescale(image, RGB<unsigned long>)
	do_rescale(image, RGB<float>)
	do_rescale(image, RGB<double>)
	throw std::runtime_error("cannot rescale this pixel type");
}

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("rescale");
	int	c;

	// parse the command line
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "d?hm:M:s:",
		longopts, &longindex)))
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
		case 's':
			scale = std::stod(optarg);
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

	// create image from rescaling adapter
	ImagePtr	outimage = rescale(image);

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
