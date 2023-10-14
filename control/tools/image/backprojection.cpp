/*
 * backprojection.cpp -- backprojection transform of an image
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <Radon.h>
#include <AstroAdapter.h>
#include <AstroConvolve.h>

using namespace astro;
using namespace astro::io;
using namespace astro::image;
using namespace astro::image::radon;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace backprojection {

/**
 * \brief display a help message for the dark program
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] infile outfile" << std::endl;
	std::cout << std::endl;
	std::cout << "compute backprojection transform of <infile> image and write it to <outfile>"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug              "
		"increase debug level" << std::endl;
	std::cout << "    -h,--height=<height>    "
		"divide 180 degrees in <height> steps" << std::endl;
	std::cout << "    -w,--width=<width>      width of the backprojection transform image"
		<< std::endl;
	std::cout << "    -f,--filter             also filter the backprojekction" << std::endl;
	std::cout << "    -h,-?,--help            "
		"show this help message" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "width",		required_argument,	NULL,	'w' }, /* 0 */
{ "height",		required_argument,	NULL,	'h' }, /* 1 */
{ "debug",		no_argument,		NULL,	'd' }, /* 2 */
{ "filter",		no_argument,		NULL,	'f' }, /* 3 */
{ "help",		no_argument,		NULL,	'?' }, /* 4 */
{ NULL,			0,			NULL,	0   }
};

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a number of images from a CCD and produces a dark image
 * from them.
 */
int	main(int argc, char *argv[]) {
	int	width = -1;
	int	height = -1;
	int	c;
	int	longindex;
	bool	filter = false;
	while (EOF != (c = getopt_long(argc, argv, "dw:h:f?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'f':
			filter = true;
			break;
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// next two arguments must be given: infile outfile
	if ((argc - optind) != 2) {
		std::cerr << "wrong number of arguments" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	infile = argv[optind++];
	std::string	outfile = argv[optind++];

	// read the input image
	FITSin	in(infile);
	ImagePtr	radonptr = in.read();
	DoubleAdapter	radon(radonptr);
	Image<double>	rawradon(radon);

	// if width or height are not set, we set them from the image
	if (width < 0) {
		width = radonptr->size().width();
	}
	if (height < 0) {
		height = radonptr->size().height();
	}

	// perform the backprojection transform
	ImageSize	backprojectionsize(width, height);
	BackProjection	backprojection(backprojectionsize, rawradon);
	Image<double>	backprojectionimage(backprojection);

	// if filtering is not required, return the image
	if (!filter) {
		io::FITSoutfile<double>	out(outfile);
		out.setPrecious(false);
		out.write(backprojectionimage);
		return EXIT_SUCCESS;
	}

	// perform the filtering
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filtering required");
	FourierImage	fi(backprojectionimage);
	FourierImage	filterimage(backprojectionimage.size());
	int	w = filterimage.size().width();
	int	W = w / 2;
	int	h = filterimage.size().height();
	int	rmax = ((W < h) ? W : h) / 2;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"fourier image has size %d x %d, rmax = %d", w, h, rmax);
	for (int x = 0; x < w / 2; x++) {
		for (int y = 0; y < h; y++) {
			double	r = 0;
			double	r00 = hypot(x, y);
			double	r01 = hypot(x, h - y);
			//double	r10 = hypot(W - x, y);
			//double	r11 = hypot(W - x, h - y);
			if (r00 < rmax) {
				r = r00;
			}
			if (r01 < rmax) {
				r = r01;
			}
			//if (r10 < rmax) {
			//	r = r10;
			//}
			//if (r11 < rmax) {
			//	r = r11;
			//}
			filterimage.pixel(2 * x, y) = r;
			filterimage.pixel(2 * x + 1, y) = 0;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter function initialized");
	FourierImagePtr	fourierfiltered = fi * filterimage;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fourier function multiplied");
	ImagePtr	filtered = fourierfiltered->inverse();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "multiplied");
	io::FITSout	out(outfile);
	out.setPrecious(false);
	out.write(filtered);

	return EXIT_SUCCESS;
}

} // namespace dark
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::backprojection::main>(argc, argv);
}
