/*
 * makedark.cpp -- create a dark frame from a set of FITS images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroIO.h>
#include <AstroUtils.h>

using namespace astro;
using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;

namespace astro {
namespace app {
namespace makedark {

/**
 * \brief Display usage help for the makedark program
 *
 * \brief progname	Path to the program (likek argv[0])
 */
void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] darkimages"
		<< std::endl;
	std::cout << std::endl;
	std::cout <<"Computes a consolidated dark image from a set of images"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "    -d,--debug                    increase debug level" << std::endl;
	std::cout << "    -h,-?,--help                  show this help message" << std::endl;
	std::cout << "    -o,--outfile=<outfile.fits>   filename of the output dark image"
		<< std::endl;
}

static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,	'd' }, /* 0 */
{ "help",		no_argument,		NULL,	'h' }, /* 1 */
{ "outfile",		required_argument,	NULL,	'o' }, /* 2 */
{ NULL,			0,			NULL,	 0  }, /* 3 */
};

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a list of image names on the command line, reads them,
 * and produces a dark image from them.
 */
int	main(int argc, char *argv[]) {
	char	*outfilename = NULL;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "do:h?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'o':
			outfilename = optarg;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
			break;
		}

	if (argc <= optind) {
		std::string	msg("no images specified");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		std::cerr << "no image file arguments specified" << std::endl;
		throw std::runtime_error(msg);
	}

	ImageSequence	images;
	for (; optind < argc; optind++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "reading file %s", argv[optind]);
		std::string	name(argv[optind]);
		FITSin	infile(name);
		ImagePtr	image = infile.read();
		images.push_back(image);
	}

	DarkFrameFactory	dff;
	ImagePtr	dark = dff(images);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image %d x %d generated",
		dark->size().width(), dark->size().height());

	if (outfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "outfile: %s", outfilename);
		unlink(outfilename);
		FITSout	outfile(outfilename);
		outfile.setPrecious(false); // allow overwriting
		outfile.write(dark);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image written to %s",
			outfilename);
	}

	return EXIT_SUCCESS;
}

} // namespace makedark
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::makedark::main>(argc, argv);
}
