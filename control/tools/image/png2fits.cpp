/*
 * png2fits.cpp -- convert JPEG Images to FITS
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include <AstroUtils.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <getopt.h>
#include <AstroDebug.h>
#include <png.h>

namespace astro {
namespace app {
namespace png2fits {

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -d ] pngfile fitsfile";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "convert a PNG image into FITS format" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug      enable debug messages" << std::endl;
	std::cout << " -h,-?,--help    display this help message and exit";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ NULL,		0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	int     c;
	int     longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	// get the file name arguments
	if (optind >= argc) {
		std::cerr << "missing JPG file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	pngfilename(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "missing FITS file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	fitsfilename(argv[optind++]);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert %s to %s", pngfilename.c_str(),
		fitsfilename.c_str());

	// open the image files
	image::PNG	png;
	image::ImagePtr	image = png.readPNG(pngfilename);
	io::FITSout	out(fitsfilename);
	out.write(image);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace png2fits 
} // namespace app
} // namespace astro

int     main(int argc, char *argv[]) {
        return astro::main_function<astro::app::png2fits::main>(argc, argv);
}

