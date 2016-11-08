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
#include <AstroCalibration.h>
#include <AstroIO.h>
#include <AstroDemosaic.h>
#include <AstroImager.h>

using namespace astro;
using namespace astro::io;
using namespace astro::calibration;

namespace astro {
namespace app {
namespace clamp {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "min",	required_argument,	NULL,		'm' }, /* 2 */
{ "max",	required_argument,	NULL,		'M' }, /* 3 */
{ "help",	no_argument,		NULL,		'h' }, /* 4 */
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
	std::cout << "  -f,--force    force overwriting of output file"
		<< std::endl;
	std::cout << "  -m,--min=min  clamp the image values to at least <min>"
		<< std::endl;
	std::cout << "  -M,--max=max  clamp the image values to at most <max>"
		<< std::endl;
	std::cout << "  -d,--debug    increase debug level" << std::endl;
	std::cout << "  -h,-?,--help  show this help message" << std::endl;
}

/**
 * \brief Main function in astro namespace
 */
int	main(int argc, char *argv[]) {
	int	c;
	double	minvalue = -1;
	double	maxvalue = -1;
	bool	force = false;

	// parse the command line
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dD:F:?hm:M:bi", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
			break;
		case 'm':
			minvalue = atof(optarg);
			break;
		case 'M':
			maxvalue = atof(optarg);
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
	outfile.setPrecious(!force);
	outfile.write(image);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace clamp
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::clamp::main>(argc, argv);
}
