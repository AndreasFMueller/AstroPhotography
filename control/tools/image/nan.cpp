/*
 * nan.cpp -- turn all NaN into zero
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
namespace nan {

static void	usage(const char *progname) {
        Path    path(progname);
        std::cout << "usage: " << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "    " << path.basename() << " [ -dh? ] infile outfile";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "remove nans from a float image" << std::endl;
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

#define do_nanzero(Pixel)						\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel >*>(&*image);	\
	if (NULL != img) {						\
		adapter::NaNzeroAdapter<Pixel >	nza(*img);		\
		outimage = ImagePtr(new Image<Pixel >(nza));		\
	}								\
}


int	main(int argc, char *argv[]) {

	debug_set_ident("nan");
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

	// next two arguments are file names
	if (optind >= argc) {
		std::cerr << "source file argument missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	infilename(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "destination file argument missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfilename(argv[optind++]);

	// read the input image
	io::FITSin	in(infilename);
	ImagePtr	image = in.read();

	// out image
	ImagePtr	outimage;

	// fix the image
	do_nanzero(float)
	do_nanzero(double)
	do_nanzero(RGB<float>)
	do_nanzero(RGB<double>)

	// write the output image
	io::FITSout	out(outfilename);
	out.write(outimage);

	return EXIT_SUCCESS;
}

} // namespace nan
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::nan::main>(argc, argv);
}
