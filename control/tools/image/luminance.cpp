/*
 * luminance.cpp -- program to balance color statistics
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <AstroAdapter.h>

namespace astro {
namespace app {
namespace luminance {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ NULL,		0,			NULL,		 0  }
};

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -dh?f ] infile outfile";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug             increase debug level";
	std::cout << std::endl;
	std::cout << "  -f,--force             force overwriting of existing files";
	std::cout << std::endl;
	std::cout << "  -h,--help              show this help message and exit";
	std::cout << std::endl;
}

template<typename Pixel, typename T>
ImagePtr	luminance(const ConstImageAdapter<Pixel>& image) {
	adapter::LuminanceAdapter<Pixel, T>	luminance(image);
	return ImagePtr(new Image<T>(luminance));
}

#define do_luminance(image, Pixel, T) {			\
	Image<Pixel>	*imagep				\
		= dynamic_cast<Image<Pixel >*>(&*image);\
	if (NULL != imagep) {				\
		return luminance<Pixel, T>(*imagep);	\
	}						\
}

ImagePtr	luminance(ImagePtr image) {
	do_luminance(image, unsigned char, unsigned char)
	do_luminance(image, unsigned short, unsigned short)
	do_luminance(image, unsigned int, unsigned int)
	do_luminance(image, unsigned long, unsigned long)
	do_luminance(image, float, float)
	do_luminance(image, double, double)
	do_luminance(image, RGB<unsigned char>, unsigned char)
	do_luminance(image, RGB<unsigned short>, unsigned short)
	do_luminance(image, RGB<unsigned int>, unsigned int)
	do_luminance(image, RGB<unsigned long>, unsigned long)
	do_luminance(image, RGB<float>, float)
	do_luminance(image, RGB<double>, double)
	std::string	msg = stringprintf("cannot get luminance for %s pixels",
		demangle(image->pixel_type().name()).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}


int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	force = false;
	while (EOF != (c = getopt_long(argc, argv, "dh?f", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// make sure we have at least 2 files
	if (optind >= argc) {
		std::cerr << "must specify image to get luminance" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	infile(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "must specify output file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfile(argv[optind++]);

	// open the input file
	io::FITSin	in(infile);
	ImagePtr	image = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s-image of type %s", 
		image->size().toString().c_str(),
		demangle(image->pixel_type().name()).c_str());

	// perform color balance
	ImagePtr	outimage = luminance(image);

	// find out whether the file exists
	io::FITSout	out(outfile);
	if (out.exists()) {
		if (force) {
			out.unlink();
		} else {
			std::cerr << "file " << outfile << " exists" << std::endl;
			return EXIT_FAILURE;
		}
	}

	// write the file
	out.write(outimage);

	return EXIT_SUCCESS;
}

} // namespace luminance
} // namespace app
} // namespace astro

int     main(int argc, char *argv[]) {
        return astro::main_function<astro::app::luminance::main>(argc, argv);
}

