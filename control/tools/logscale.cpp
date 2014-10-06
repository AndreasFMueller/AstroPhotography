/**
 * logimage.cpp -- take binary logarithm of all pixels of an image
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

/**
 * \brief usage
 */
void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ] infile outfile"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d        increase debug level" << std::endl;
	std::cout << "  -h, -?    show this help message" << std::endl;
}

/**
 * \brief Main function in astro namespace
 */
int	logscale_main(int argc, char *argv[]) {
	int	c;

	// parse the command line
	while (EOF != (c = getopt(argc, argv, "d?h")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
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

	// convert pixels according to luminance
#if 0
	Image<RGB<double> >	*imageptr
		= dynamic_cast<Image<RGB<double> > *>(&*image);
	LuminanceAdapter<RGB<double> >	la(*imageptr);
#endif
	Image<double >	*imageptr
		= dynamic_cast<Image<double > *>(&*image);
	LuminanceAdapter<double, double>	la(*imageptr);

	// create image from log adapter
	FunctionAdapter<double>	fa(la, log2);
	Image<double>	*logimage = new Image<double>(fa);
	ImagePtr	outimage(logimage);

	// after all the calibrations have been performed, write the output
	// file
	FITSout	outfile(outfilename);
	outfile.write(outimage);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::logscale_main>(argc, argv);
}
