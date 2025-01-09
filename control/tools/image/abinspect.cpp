/*
 * abinspect.cpp -- 
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

static void	usage(char *progname) {
}

static struct option	options[] = {
{ "debug",		no_argument,		NULL,	'd' },
{ "gap",		required_argument,	NULL,	'g' },
{ "help",		no_argument,		NULL,	'h' },
{ "size",		required_argument,	NULL,	's' },
{ NULL,			0,			NULL,	 0  }
};

int	main(int argc, char *argv[]) {
	int	gapwidth = 3;
	int	patchsize = 200;

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
		case 'h':
			usage(argv[0]);
			break;
		case 's':
			patchsize = std::stoi(optarg);
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
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no output image file name specified");
		std::cerr << "no output image file argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfilename(argv[optind++]);

	// read the image
	io::FITSin	infile(infilename);
	ImagePtr	inimage = infile.read();

	// compute the window sizes
	int	l = 3 * patchsize + 2 * gapwidth;
	if ((l > inimage->size().width()) || (l > inimage->size().height())) {
		std::cerr << "input image too small, must be at least ";
		std::cerr << l << "x" << l << std::endl;
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
