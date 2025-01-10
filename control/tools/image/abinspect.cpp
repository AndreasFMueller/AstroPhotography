/*
 * abinspect.cpp -- aberration inspector
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroImage.h>
#include <AstroIO.h>

namespace astro {
namespace app {
namespace abinspect {

/**
 * \brief Help message
 *
 * \param progname	name of the program
 */
static void	usage(char *progname) {
	std::cout << "construct an aberration inspector for an image"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "    " << progname << " [ options ] infile outfile"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug        show debug messages" << std::endl;
	std::cout << "  -g,--gap=<4>	  width of the gap between parts"
		<< std::endl;
	std::cout << "  -?,--help      show this help message and exit"
		<< std::endl;
	std::cout << "  -w,--width=<w>    width of each part (must be even)"
		<< std::endl;
	std::cout << "  -h,--height=<h>   height of each part (must be even)"
		<< std::endl;
}

static struct option	options[] = {
{ "debug",		no_argument,		NULL,	'd' },
{ "gap",		required_argument,	NULL,	'g' },
{ "help",		no_argument,		NULL,	'?' },
{ "height",		required_argument,	NULL,	'h' },
{ "width",		required_argument,	NULL,	'w' },
{ NULL,			0,			NULL,	 0  }
};

/**
 * \brief Main function for the aberration inspector program
 *
 * \param argc		the number of arguments
 * \param argv		the list of arguments
 */
int	main(int argc, char *argv[]) {
	int	gapwidth = 3;
	int	patchwidth = 200;
	int	patchheight = 150;

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dg:?hs:",
		options, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'g':
			gapwidth = std::stoi(optarg);
			break;
		case '?':
			usage(argv[0]);
			break;
		case 'w':
			patchwidth = std::stoi(optarg);
			break;
		case 'h':
			patchheight = std::stoi(optarg);
			break;
		}

	// next two arguments are file names
	if (argc <= optind) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image specified");
		std::cerr << "no image file argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	infilename(argv[optind++]);
	if (argc <= optind) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"no output image file name specified");
		std::cerr << "no output image file argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfilename(argv[optind++]);

	// read the image
	io::FITSin	infile(infilename);
	ImagePtr	inimage = infile.read();

	// compute the window sizes
	int	lw = 3 * patchwidth + 2 * gapwidth; 
	int	lh = 3 * patchheight + 2 * gapwidth;
	if ((lw > inimage->size().width()) || (lh > inimage->size().height())) {
		std::cerr << "input image too small, must be at least ";
		std::cerr << patchwidth << "x" << patchheight << std::endl;
		return EXIT_FAILURE;
	}

	// create the output image and initialize it to black
	

	// copy windows into the image

	// write the image to a file
	io::FITSout	outfile(outfilename);
	outfile.setPrecious(false);

	return EXIT_SUCCESS;
}

} // namespace abinspect
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main_function<astro::app::abinspect::main>(argc,
			argv);
	} catch (const std::exception& x) {
		std::cerr << astro::stringprintf(
			"aberration inspector throws exception: %s", x.what());
	} catch (...) {
		std::cerr << "aberration inspector terminated by unknown "
			"exceptionm" << std::endl;
	}
	return EXIT_FAILURE;
}
