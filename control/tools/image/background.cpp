/*
 * background.cpp -- find and subtract the background from an image
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <AstroBackground.h>

using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace background {

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage:" << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <infile>"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -a,--alpha=<alpha>      use this value for background extraction"
		<< std::endl;
	std::cout << "  -d,--debug              increase debug level"
		<< std::endl;
	std::cout << "  -f,--force              force overwriting of the output file"
		<< std::endl;
	std::cout << "  -h,--help               display this help message"
		 << std::endl;
	std::cout << "  -D,--degree=<d>         degree of the polynomial, valid values " << std::endl;
	std::cout << "                          are 0, 1, 2 or 4" << std::endl;
	std::cout << "  -o,--outfile=<file>     write corrected image to the "
		"FITS file named <file>" << std::endl;
}

static struct option	longopts[] = {
{ "alpha",	required_argument,	NULL,		'a' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "degree",	required_argument,	NULL,		'D' }, /* 4 */
{ "force",	no_argument,		NULL,		'f' }, /* 2 */
{ "help",	no_argument,		NULL,		'h' }, /* 3 */
{ "outfile",	required_argument,	NULL,		'o' }, /* 4 */
{ NULL,		0,			NULL,		0   }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	std::string	outfilename;
	bool	force = false;
	float	alpha = 0.001;
	BackgroundExtractor::functiontype	type
		= BackgroundExtractor::QUADRATIC;
	while (EOF != (c = getopt_long(argc, argv, "a:dfho:", longopts,
                &longindex)))
                switch (c) {
		case 'a':
			alpha = std::stof(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'D':
			switch (std::stoi(optarg)) {
			case 0:
				type = BackgroundExtractor::CONSTANT;
				break;
			case 1:
				type = BackgroundExtractor::LINEAR;
				break;
			case 2:
				type = BackgroundExtractor::QUADRATIC;
				break;
			case 4:
				type = BackgroundExtractor::DEGREE4;
				break;
			}
			break;
		case 'f':
			force = true;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'o':
			outfilename = std::string(optarg);
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// get the file name
	if (argc <= optind) {
		throw std::runtime_error("input file name missing");
	}
	std::string	infilename(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing image %s",
		infilename.c_str());

	// read the input file
	FITSin	infile(infilename);
	ImagePtr	image = infile.read();
	ImagePtr	outimage;

	// prepare a background extractor
	BackgroundExtractor	extractor(alpha);

	// if this is a mono image, we just use luminance for background
	// extraction
	switch (image->planes()) {
	case 1:	{
		// make image accessible as an image with float pixels
		ConstPixelValueAdapter<float>	from(image);

		// get the background
		Background<float>	bg = extractor(image->center(), true,
						type, from);

		// subtract the background
		BackgroundFunctionAdapter	bfa(from, bg.G());

		// write the result to the output
		outimage = ImagePtr(new Image<float>(bfa));
		}
		break;
	case 3:	{
		// make image accessible as an RGB<float> image
		ConstPixelValueAdapter<RGB<float> >	from(image);

		// get the background
		Background<float>	bg = extractor(image->center(), true,
						type, from);

		// subtract the background
		BackgroundSubtractionAdapter	bsa(from, bg);

		// write the result to the output
		outimage = ImagePtr(new Image<RGB<float> >(bsa));
		}
		break;
	default:
		std::string	msg = stringprintf("don't know how to handle "
			"background for images with %d planes",
			image->planes());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// we give up here, because we don't want to write the changed file
	if (0 == outfilename.size()) {
		return EXIT_SUCCESS;
	}


	FITSout	outfile(outfilename);
	outfile.setPrecious(!force);
	outfile.write(outimage);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace background
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::background::main>(argc, argv);
}
