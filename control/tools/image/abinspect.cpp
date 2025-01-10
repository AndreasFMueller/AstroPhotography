/*
 * abinspect.cpp -- aberration inspector
 *
 * (c) 2025 Prof Dr Andreas Müller
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroImage.h>
#include <AstroAdapter.h>
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

#define ab_typed(Pixel)							\
{									\
	adapter::AberrationInspectorFactory<Pixel>	abf(targetsize);\
	abf.gap(gapwidth);						\
	ConstImageAdapter<Pixel>	*inp				\
		= dynamic_cast<ConstImageAdapter<Pixel >*>(&*inimage);	\
	if (NULL != inp) {						\
		adapter::WindowsAdapter<Pixel >	*wa = abf(*inp, false);	\
		Image<Pixel >	*outp = new Image<Pixel>(*wa);		\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new image constructed");\
		out = ImagePtr(outp);					\
		delete wa;						\
	}								\
}

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
			return EXIT_SUCCESS;
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

	// compute the window sizes
	io::FITSin	infile(infilename);
	ImagePtr	inimage = infile.read();


	// make sure the patches are at most as large as the source image
	if ((patchwidth > inimage->size().width())
		|| (patchheight > inimage->size().height())) {
		std::cerr << "input image too small, must be at least ";
		std::cerr << patchwidth << "x" << patchheight << std::endl;
		return EXIT_FAILURE;
	}

	// compute the window sizes
	int	lw = 3 * patchwidth + 2 * gapwidth;
	int	lh = 3 * patchheight + 2 * gapwidth;
	ImageSize	targetsize(lw, lh);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target image size: %s",
		targetsize.toString().c_str());

	// create the output image and initialize it to black
	ImagePtr	out;
	ab_typed(unsigned char)
	ab_typed(unsigned short)
	ab_typed(unsigned int)
	ab_typed(unsigned long)
	ab_typed(float)
	ab_typed(double)
	ab_typed(RGB<unsigned char>)
	ab_typed(RGB<unsigned short>)
	ab_typed(RGB<unsigned int>)
	ab_typed(RGB<unsigned long>)
	ab_typed(RGB<float>)
	ab_typed(RGB<double>)

	if (!out) {
		std::cerr << "could not construct ab inspector image"
			<< std::endl;;
		return EXIT_FAILURE;
	}

	// write the image to a file
	io::FITSout	outfile(outfilename);
	outfile.setPrecious(false);
	outfile.write(out);

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
