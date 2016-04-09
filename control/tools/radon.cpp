/*
 * radon.cpp -- radon transform of an image
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <Radon.h>
#include <AstroAdapter.h>

using namespace astro;
using namespace astro::io;
using namespace astro::image;
using namespace astro::image::radon;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace radon {

/**
 * \brief display a help message for the dark program
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] infile outfile" << std::endl;
	std::cout << std::endl;
	std::cout << "compute radon transform of <infile> image and write it to <outfile>"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug                "
		"increase debug level" << std::endl;
	std::cout << "    -h,--height=<height>      "
		"divide 180 degrees in <height> steps" << std::endl;
	std::cout << "    -w,--width=<width>      width of the radon transform image"
		<< std::endl;
	std::cout << "    -h,-?,--help              "
		"show this help message" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "width",		required_argument,	NULL,	'w' }, /* 0 */
{ "height",		required_argument,	NULL,	'h' }, /* 1 */
{ "debug",		no_argument,		NULL,	'd' }, /* 2 */
{ "help",		no_argument,		NULL,	'?' }, /* 2 */
{ "full",		no_argument,		NULL,	'f' },
{ NULL,			0,			NULL,	0   }
};

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a number of images from a CCD and produces a dark image
 * from them.
 */
int	main(int argc, char *argv[]) {
	int	width = 1024;
	int	height = 512;
	bool	full = false;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dw:h:f?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			full = true;
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
			break;
		}

	// next two arguments must be given: infile outfile
	if ((argc - optind) != 2) {
		std::cerr << "wrong number of arguments" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	infile = argv[optind++];
	std::string	outfile = argv[optind++];

	// read the input image
	FITSin	in(infile);
	ImagePtr	imageptr = in.read();
	DoubleAdapter	image(imageptr);
	Image<double>	rawimage(image);
	FITSoutfile<double>	outimage("radonimage.fits");
	outimage.setPrecious(false);
	outimage.write(rawimage);

	// perform the radon transform
	if (full) {
		height = height / 2;
	}
	ImageSize	radonsize(width, height);
	Image<double>	*radonimage = NULL;
	if (full) {
		RadonTransform	radon(radonsize, rawimage);
		std::cout << "transform complete: " << radon.getSize()
			<< std::endl;
		radonimage = new Image<double>(radon);
	} else {
		RadonAdapter	radon(radonsize, rawimage);
		std::cout << "transform complete: " << radon.getSize()
			<< std::endl;
		radonimage = new Image<double>(radon);
	}
	std::cout << "copy complete" << std::endl;

	// write the result
	io::FITSoutfile<double>	out(outfile);
	out.setPrecious(false);
	out.write(*radonimage);
	delete radonimage;

	return EXIT_SUCCESS;
}

} // namespace dark
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::radon::main>(argc, argv);
}
