/**
 * calibrate images using darks and flats
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <debug.h>
#include <Format.h>
#include <iostream>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroIO.h>
#include <AstroDemosaic.h>

using namespace astro;
using namespace astro::io;
using namespace astro::calibration;

namespace astro {

/**
 * \brief usage
 */
void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ] infile outfile"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -D dark   use image file <dark> for dark correction"
		<< std::endl;
	std::cout << "  -F flat   use image file <flat> for flat correction"
		<< std::endl;
	std::cout << "  -m min    clamp the image values to at least <min>"
		<< std::endl;
	std::cout << "  -M max    clamp the image values to at most <max>"
		<< std::endl;
	std::cout << "  -d        increase debug level" << std::endl;
	std::cout << "  -n, -?    show this help message" << std::endl;
}

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	int	c;
	const char	*darkfilename = NULL;
	const char	*flatfilename = NULL;
	double	minvalue = -1;
	double	maxvalue = -1;
	bool	demosaic = false;

	// parse the command line
	while (EOF != (c = getopt(argc, argv, "dD:F:?hm:M:b")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'D':
			darkfilename = optarg;
			break;
		case 'F':
			flatfilename = optarg;
			break;
		case 'm':
			minvalue = atof(optarg);
			break;
		case 'M':
			maxvalue = atof(optarg);
			break;
		case 'b':
			demosaic = true;
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

	// if we have a dark correction, apply it
	if (NULL != darkfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark correct: %s", darkfilename);
		FITSin	darkin(darkfilename);
		ImagePtr	dark = darkin.read();
		DarkCorrector	corrector(dark);
		corrector(image);
	}

	// if we have a flat file, we perform flat correction
	if (NULL != flatfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flat correction: %s",
			flatfilename);
		FITSin	flatin(flatfilename);
		ImagePtr	flat = flatin.read();
		FlatCorrector	corrector(flat);
		corrector(image);
	}

	// if minvalue or maxvalue are set, clamp the image values
	if ((minvalue >= 0) || (maxvalue >= 0)) {
		if (minvalue < 0) {
			minvalue = 0;
		}
		if (maxvalue < 0) {
			maxvalue = std::numeric_limits<double>::infinity();
		}
		Clamper	clamp(minvalue, maxvalue);
		clamp(image);
	}

	// after all the calibrations have been performed, write the output
	// file
	FITSout	outfile(outfilename);

	// if demosaic is requested we do that now
	if (demosaic) {
		ImagePtr	demosaiced = demosaic_bilinear(image);
		outfile.write(demosaiced);
	} else {
		outfile.write(image);
	}

	// that's it
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::string	msg = stringprintf("%s terminated: %s",
			argv[0], x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		std::cerr << msg << std::endl;
	}
}
