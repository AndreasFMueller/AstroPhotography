/**
 * clamp image -- image clamping utility
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <iostream>
#include <AstroImage.h>
#include <AstroAdapter.h>
#include <AstroIO.h>
#include <AstroDemosaic.h>
#include <AstroImager.h>

using namespace astro;
using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace clamp {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "x",		required_argument,	NULL,		'x' }, /* 2 */
{ "y",		required_argument,	NULL,		'y' }, /* 3 */
{ "width",	required_argument,	NULL,		'w' }, /* 3 */
{ "height",	required_argument,	NULL,		'h' }, /* 3 */
{ "help",	no_argument,		NULL,		'H' }, /* 4 */
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
	std::cout << "  -f,--force       force overwriting of output file"
		<< std::endl;
	std::cout << "  -x,--x=<x>       lower left corner x-coordinate of crop area"
		<< std::endl;
	std::cout << "  -y,--y=<y>       lower left corner y-coordinate of crop area"
		<< std::endl;
	std::cout << "  -w,--width=<w>   width of crop area" << std::endl;
	std::cout << "  -h,--height=<h>  height of crop area" << std::endl;
	std::cout << "  -d,--debug       increase debug level" << std::endl;
	std::cout << "  -H,-?,--help     show this help message" << std::endl;
}

#define	window_adapter(image, pixel)					\
{									\
	Image<pixel>	*p = dynamic_cast<Image<pixel>*>(&*image);	\
	if (NULL != p) {						\
		WindowAdapter<pixel>	wa(*p, rectangle);		\
		result = ImagePtr(new Image<pixel>(wa));		\
	}								\
}

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	int	c;
	bool	force = false;
	int	xoffset = 0;
	int	yoffset = 0;
	int	width = -1;
	int	height = -1;

	// parse the command line
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dD:F:?hm:M:b", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
			break;
		case 'x':
			xoffset = std::stoi(optarg);
			break;
		case 'y':
			yoffset = std::stoi(optarg);
			break;
		case 'w':
			width = std::stoi(optarg);
			break;
		case 'h':
			height = std::stoi(optarg);
			break;
		case '?':
		case 'H':
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "crop %s to %s",
		infilename.c_str(), outfilename.c_str());

	// read the infile
	FITSin	infile(infilename);
	ImagePtr	image = infile.read();

	// ensure we have valid width and height
	if (image->size().width() <= xoffset) {
		throw std::runtime_error("x offset too large");
	}
	if ((image->size().width() - xoffset) < width) {
		width = image->size().width() - xoffset;
	}
	if (image->size().height() <= yoffset) {
		throw std::runtime_error("y offset too large");
	}
	if ((image->size().height() - yoffset) < height) {
		height = image->size().height() - yoffset;
	}
	ImageRectangle	rectangle(ImagePoint(xoffset, yoffset),
				ImageSize(width, height));

	// create an image area adapter
	ImagePtr	result;
	window_adapter(image, double);
	window_adapter(image, float);
	window_adapter(image, unsigned short);
	window_adapter(image, unsigned long);
	window_adapter(image, unsigned char);

	// after all the calibrations have been performed, write the output
	// file
	FITSout	outfile(outfilename);
	outfile.setPrecious(!force);
	outfile.write(result);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace clamp
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::clamp::main>(argc, argv);
}
