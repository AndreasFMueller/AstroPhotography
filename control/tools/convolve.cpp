/*
 * convolve.cpp --
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

int	main(int argc, char *argv[]) {
	// parse command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
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
