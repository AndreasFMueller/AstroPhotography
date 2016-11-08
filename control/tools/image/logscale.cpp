/**
 * logscale.cpp -- take binary logarithm of all pixels of an image
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
#include <cmath>
#include <typeinfo>

using namespace astro;
using namespace astro::io;
using namespace astro::calibration;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace logscale {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' },
{ "help",	no_argument,		NULL,		'h' }, /* 1 */
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
	std::cout << "  -d        increase debug level" << std::endl;
	std::cout << "  -f        force overwriting of images" << std::endl;
	std::cout << "  -h, -?    show this help message" << std::endl;
}

#if 0
template<typename T, typename S>
class RGBLogAdapter : public ConstImageAdapter<RGB<T> > {
	const ConstImageAdapter<RGB<T> >&	_image;
	LuminanceAdapter<RGB<T>,S>	_luminance;
public:
	RGBLogAdapter(const ConstImageAdapter<RGB<T> >& image)
		: ConstImageAdapter<RGB<T> >(image.getSize()), _image(image),
		  _luminance(image) {
	}
	virtual RGB<T>	pixel(int x, int y) const {
		S	l = _luminance.pixel(x, y);
		S	s = log(l) / l;
		if (s < 0) { s = 0; }
		RGB<T>	p = _image.pixel(x, y);
		RGB<T>	result(s * p.R, s * p.G, s * p.B);
		return result;
	}
	static ImagePtr	rgblog(const ConstImageAdapter<RGB<T> >& image) {
		RGBLogAdapter<T, S>	a(image);
		return ImagePtr(new Image<RGB<T> >(a));
	}
};


#define	do_rgblog(image, T, S)						\
	{								\
		Image<RGB<T> >	*imagep					\
			= dynamic_cast<Image<RGB<T> >*>(&*image);	\
		if (NULL != imagep) {					\
			return RGBLogAdapter<T,S>::rgblog(*imagep);	\
		}							\
	}

ImagePtr	rgblog(ImagePtr image) {
	do_rgblog(image, float, float)
	do_rgblog(image, double, double)
	throw std::runtime_error("cannot log image with this pixel type");
}

template<typename T>
class LogAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<T>&	_image;
public:
	LogAdapter(const ConstImageAdapter<T>& image)
		: ConstImageAdapter<T>(image.getSize()), _image(image) {
	}
	virtual T	pixel(int x, int y) const {
		return log(_image.pixel(x, y));
	}
	static ImagePtr	logimage(const ConstImageAdapter<T>& image) {
		LogAdapter<T>	l(image);
		return ImagePtr(new Image<T>(l));
	}
};

#define	do_logimage(image, Pixel)					\
	{								\
		Image<Pixel>	*imagep					\
			= dynamic_cast<Image<Pixel>*>(&*image);		\
		if (NULL != imagep) {					\
			return LogAdapter<Pixel>::logimage(*imagep);	\
		}							\
	}

ImagePtr	logimage(ImagePtr image) {
	do_logimage(image, float)
	do_logimage(image, double)
	throw std::runtime_error("cannot log image with this pixel type");
}
#endif

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	bool	force = false;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "df?h", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "log %s to %s",
		infilename.c_str(), outfilename.c_str());

	// read the infile
	FITSin	infile(infilename);
	ImagePtr	image = infile.read();

	// outfile
	ImagePtr	outimage;

	// convert pixels according to luminance
	outimage = logimage(image);
#if 0
	if (3 == image->planes()) {
		outimage = rgblog(image);
	} else {
		outimage = logimage(image);
	}
#endif

	// after all the calibrations have been performed, write the output
	// file
	FITSout	outfile(outfilename);
	if ((outfile.exists()) && (force)) {
		outfile.unlink();
	}
	outfile.write(outimage);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace logscale
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::logscale::main>(argc, argv);
}
