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
	std::cout << "  -o,--outfile=<file>     write corrected image to the "
		"FITS file named <file>" << std::endl;
}

static struct option	longopts[] = {
{ "alpha",	required_argument,	NULL,		'a' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
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
	while (EOF != (c = getopt_long(argc, argv, "a:dfho:", longopts,
                &longindex)))
                switch (c) {
		case 'a':
			alpha = std::stof(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
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
	ConstPixelValueAdapter<float>	from(image);

	// get the background
	BackgroundExtractor	extractor(alpha);
	Background<float>	bg = extractor(image->center(), true,
					BackgroundExtractor::QUADRATIC, from);

	// subtract the background
	BackgroundFunctionAdapter	bfa(from, bg.G());

	// we give up here, because we don't want to write the changed file
	if (0 == outfilename.size()) {
		return EXIT_SUCCESS;
	}

	// write the result to the output
	ImagePtr	outimage(new Image<float>(bfa));
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
