/*
 * listnan.cpp -- list the position of all nan pixels
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace app {
namespace listnan {

static void	usage(const char *progname) {
        Path    path(progname);
	std::cout << "list all nan pixel positions" << std::endl;
        std::cout << "usage: " << std::endl;
        std::cout << std::endl;
        std::cout << "    " << path.basename() << " [ -dh? ] infile";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug      show debug info" << std::endl;
	std::cout << "  -h,-?,--help    show this help message and exit";
	std::cout << std::endl;
}

/**
 * \brief Table of options
 */
static struct option    longopts[] = {
/* name         argument?               int*            int */
{ "debug",      no_argument,            NULL,           'd' }, /* 1 */
{ "help",       no_argument,            NULL,           'h' }, /* 2 */
{ NULL,         0,                      NULL,           0   }
};

static size_t	nanpixelvalues;

#define do_listnan(Pixel)						\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel >*>(&*image);	\
	if (NULL != img) {						\
		for (int x = 0; x < img->size().width(); x++) {		\
			for (int y = 0; y < img->size().height(); y++) {\
				Pixel	v = img->pixel(x, y);		\
				if (!(v == v)) {			\
					std::cout << "(" << x << ",";	\
					std::cout << y << ")";		\
					std::cout << std::endl;		\
					nanpixelvalues++;		\
				}					\
			}						\
		}							\
	}								\
}

#define do_listnanrgb(Pixel)						\
{									\
	Image<RGB<Pixel > >	*img = dynamic_cast<Image<RGB<Pixel > >*>(&*image);	\
	if (NULL != img) {						\
		for (int x = 0; x < img->size().width(); x++) {		\
			for (int y = 0; y < img->size().height(); y++) {\
				RGB<Pixel>	v = img->pixel(x, y);	\
				if (!(v.R == v.R)) {			\
					std::cout << "(" << x << ",";	\
					std::cout << y << ").R";	\
					std::cout << std::endl;		\
					nanpixelvalues++;		\
				}					\
				if (!(v.G == v.G)) {			\
					std::cout << "(" << x << ",";	\
					std::cout << y << ").G";	\
					std::cout << std::endl;		\
					nanpixelvalues++;		\
				}					\
				if (!(v.B == v.B)) {			\
					std::cout << "(" << x << ",";	\
					std::cout << y << ").B";	\
					std::cout << std::endl;		\
					nanpixelvalues++;		\
				}					\
			}						\
		}							\
	}								\
}

int	main(int argc, char *argv[]) {

	debug_set_ident("listnan");
	int     c;
	int     longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

	// next argument is a file name
	if (optind >= argc) {
		std::cerr << "source file argument missing" << std::endl;
		return EXIT_FAILURE;
	}

	// all the following arguments are file names
	// read the input image
	while (optind < argc) {
		std::string	infilename(argv[optind++]);
		std::cout << "File: " << infilename << std::endl;
		io::FITSin	in(infilename);
		ImagePtr	image = in.read();
		std::cout << "pixel type: " << demangle(image->pixel_type().name());
		std::cout << std::endl;

		// fix the image
		nanpixelvalues = 0;
		do_listnan(float)
		do_listnan(double)
		do_listnanrgb(RGB<float>)
		do_listnanrgb(RGB<double>)
		std::cout << "number of nan pixels: " << nanpixelvalues << std::endl;
	}

	return EXIT_SUCCESS;
}

} // namespace listnan
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::listnan::main>(argc, argv);
}
