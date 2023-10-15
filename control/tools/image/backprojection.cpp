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
	std::cout << "    -F,--filtered-file=<f>  write the filtered radon transform to this file" << std::endl;
	std::cout << "                            (implies the -f option)" << std::endl;
	std::cout << "    -r,--radius=<r>         set the radius for filtering" << std::endl;
	std::cout << "    -h,-?,--help            "
		"show this help message" << std::endl;
	std::cout << std::endl;
}

/**
 * \brief logarithmic image
 *
 * convert the image values to logarithmic values to make small
 * values visible
 *
 * \param image		replace pixel values of this image by logs
 */
static void	logarithmic_image (Image<double>& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "take logarithm values of image");
	const unsigned int	l = image.size().getPixels();
	for (unsigned int i = 0; i < l; i++) {
		double v = image[i];
		if (v < 1) {
			v = 0;
		} else {
			v = log(v);
		}
		image[i] = v;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "logarithmization complete");
}

/**
 * \brief Filter the radon transform
 *
 * This function performs a Fourier-filtering on every row of the input
 * image
 *
 * \param rawradon	the raw radon transform
 * \param logarithmic	whether or not to export a logarithmic version
 * \param filteredname	the name of the file to write the filtered 
 *			radon transform to
 */
static void	filter_radon(Image<double>& rawradon, bool logarithmic,
			const std::string& filteredname, double radius) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filtering radon transform, radius = %f",
		radius);
	// perform filtering on 1d Fourier transforms
	const int	w = rawradon.size().width();
	const int	w2 = w / 2;
	const int	h = rawradon.size().height();
	fftw_complex	*rspace
		= (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * w);
	fftw_complex	*fspace
		= (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * w);
	fftw_plan	forward = fftw_plan_dft_1d(w, rspace, fspace,
				-1, FFTW_ESTIMATE);
	fftw_plan	backward = fftw_plan_dft_1d(w, fspace, rspace,
				1, FFTW_ESTIMATE);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			rspace[x][0] = rawradon.pixel((x + w2) % w, y);
			rspace[x][1] = 0;
		}
		fftw_execute(forward);
		fspace[0][0] = 0;
		fspace[0][1] = 0;
#if 1
		for (int k = 1; k < w2; k++) {
			double	factor = 1.;
			if (k < radius) 
				factor = k / radius;
			fspace[k][0] *= factor;
			fspace[k][1] *= factor;
			fspace[w - k][0] *= factor;
			fspace[w - k][1] *= factor;
		}
#if 1
		if (w2 == w - w2) {
			fspace[w2][0] = 0;
			fspace[w2][1] = 0;
		}
#endif
#endif
		fftw_execute(backward);
		for (int x = 0; x < w; x++) {
			rawradon.pixel((x + w2) % w, y)
				= rspace[x][0];
				//= hypot(rspace[x][0], rspace[x][1]);
		}
	}
	if (filteredname.size() != 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"writing filtered radon transform to %s",
			filteredname.c_str());
		io::FITSoutfile<double>	out(filteredname);
		out.setPrecious(false);
		if (logarithmic) {
			Image<double>	im(rawradon);
			logarithmic_image(im);
			out.write(im);
		} else {
			out.write(rawradon);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"filtered image written");
	}
}

static struct option	longopts[] = {
{ "width",		required_argument,	NULL,	'w' }, /* 0 */
{ "height",		required_argument,	NULL,	'h' }, /* 1 */
{ "debug",		no_argument,		NULL,	'd' }, /* 2 */
{ "filter",		no_argument,		NULL,	'f' }, /* 3 */
{ "filtered-file",	required_argument,	NULL,	'F' }, /* 4 */
{ "log",		no_argument,		NULL,	'l' }, /* 5 */
{ "help",		no_argument,		NULL,	'?' }, /* 6 */
{ "radius",		required_argument,	NULL,	'r' }, /* 7 */
{ NULL,			0,			NULL,	0   }
};

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a number of images from a CCD and produces a dark image
 * from them.
 */
int	main(int argc, char *argv[]) {
	std::string	filteredname;
	int	width = -1;
	int	height = -1;
	int	c;
	int	longindex;
	bool	filter = false;
	bool	logarithmic = false;
	double	radius = 0;
	while (EOF != (c = getopt_long(argc, argv, "dw:h:fF:lr:?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			height = std::stoi(optarg);
			break;
		case 'w':
			width = std::stoi(optarg);
			break;
		case 'f':
			filter = true;
			break;
		case 'F':
			filteredname = std::string(optarg);
			filter = true;
			break;
		case 'l':
			logarithmic = true;
			break;
		case 'r':
			radius = std::stoi(optarg);
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

	// new filter implementation
	if (filter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filtering: %s file=%s",
			(logarithmic) ? "logarithmic" : "linear",
			filteredname.c_str());
		filter_radon(rawradon, logarithmic, filteredname,
			(radius > 0) ? radius : 300);
	}

	// if width or height are not set, we set them from the image
	if (width < 0) {
		width = radonptr->size().width();
	}
	if (height < 0) {
		height = radonptr->size().height();
	}

#if 1
	// perform the backprojection transform
	debug(LOG_DEBUG, DEBUG_LOG, 0, "perform back projection");
	ImageSize	backprojectionsize(width, height);
	BackProjection	backprojection(backprojectionsize, rawradon);
	Image<double>	backprojectionimage(backprojection);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "back projection completed");

	// if filtering is not required, return the image
	io::FITSoutfile<double>	out(outfile);
	out.setPrecious(false);
	out.write(backprojectionimage);
#endif
	return EXIT_SUCCESS;
}

} // namespace dark
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::backprojection::main>(argc, argv);
}
