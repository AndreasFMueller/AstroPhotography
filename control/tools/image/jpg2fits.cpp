/*
 * jpg2fits.cpp -- convert JPEG Images to FITS
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include <AstroImage.h>
#include <AstroIO.h>
#include <getopt.h>
#include <AstroUtils.h>
#include <JPEG.h>
#include <AstroDebug.h>

namespace astro {
namespace app {
namespace jpg2fits {

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -d ] jpgfile fitsfile";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "convert a JPG image into FITS format" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug      enable debug messages" << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ NULL,		0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	int     c;
	int     longindex;
	while (EOF != (c = getopt_long(argc, argv, "d", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}
	}

	// get the file name arguments
	if (optind >= argc) {
		std::cerr << "missing JPG file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	jpgfilename(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "missing FITS file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	fitsfilename(argv[optind++]);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert %s to %s", jpgfilename.c_str(),
		fitsfilename.c_str());

	// open the image files
	image::JPEG	jpeg;
	image::ImagePtr	image = jpeg.readJPEG(jpgfilename);
	io::FITSout	out(fitsfilename);
	out.write(image);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace jpg2fits 
} // namespace app
} // namespace astro

int     main(int argc, char *argv[]) {
        return astro::main_function<astro::app::jpg2fits::main>(argc, argv);
}

