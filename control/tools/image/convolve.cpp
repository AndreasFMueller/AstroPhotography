/*
 * convolve.cpp -- command line tool to compute convolution of tw images
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroImage.h>
#include <AstroDebug.h>
#include <AstroConvolve.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace app {
namespace convolve {

/**
 * \brief display a help message
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename()
		<< " [ options ] <in1.fits> <in2.fits> <out.fits>" << std::endl;
	std::cout << std::endl;
	std::cout << "Compute the convolution of the two images in files "
		"<in1.fits> and <in2.fits>" << std::endl;
	std::cout << "and store the result in the file <out.fits>."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug    increase debug level" << std::endl;
	std::cout << "    -h,--help    increase debug level" << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,	NULL,	'd' }, /* 0 */
{ "help",	no_argument,	NULL,	'h' }, /* 1 */
{ NULL,		0,		NULL,	 0  }, /* 2 */
};

/**
 * \brief main function for the convolve program
 */
int	main(int argc, char *argv[]) {
	// parse command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}

	// next two arguments must be filenames
	if ((argc - optind) != 3) {
		std::cerr << "need exactly three file name arguments"
			<< std::endl;
		return EXIT_FAILURE;
	}
	const char	*in1filename = argv[optind++];
	const char	*in2filename = argv[optind++];
	const char	*outfilename = argv[optind++];

	// read the image from the file
	FITSin	in1file(in1filename);
	ImagePtr	image1 = in1file.read();
	Image<double>	*img1 = dynamic_cast<Image<double> *>(&*image1);
	if (NULL == img1) {
		debug(LOG_ERR, DEBUG_LOG, 0, "can only convolve double images");
	}
	ConvolutionResult	factor1(*img1, ImagePoint(0, 0));

	FITSin	in2file(in2filename);
	ImagePtr	image2 = in2file.read();
	Image<double>	*img2 = dynamic_cast<Image<double> *>(&*image2);
	if (NULL == img2) {
		debug(LOG_ERR, DEBUG_LOG, 0, "can only convolve double images");
	}
	ConvolutionResult	factor2(*img2, ImagePoint(0, 0));

	// compute the convolution
	ConvolutionResultPtr	result = factor1 * factor2;

	// write the result image
	FITSout	outfile(outfilename);
	outfile.setPrecious(false);
	outfile.write(result->image());

	// that's it
	return EXIT_SUCCESS;
}

} // namespace convolve
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::convolve::main>(argc, argv);
}
