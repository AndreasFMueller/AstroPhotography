/*
 * destar.cpp -- destar an image removes small stars using a median filter
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <AstroAdapter.h>

namespace astro {
namespace app {
namespace destar {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "radius",     required_argument,	NULL,		'r' },
{ NULL,		0,			NULL,		 0  }
};

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -dh?f ] infile outfile";
	std::cout << std::endl;
	std::cout << "remove stars from an image by applying a spacial median filter to disks of" << std::endl;
	std::cout << "of radius specified with the -r option." << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug             increase debug level";
	std::cout << std::endl;
	std::cout << "  -f,--force             force overwriting of existing files";
	std::cout << std::endl;
	std::cout << "  -h,--help              show this help message and exit";
	std::cout << std::endl;
	std::cout << "  -r,--radius=<r>        destar radius";
	std::cout << std::endl;
}

template<typename T>
ImagePtr	destar(const ConstImageAdapter<T>& image, int radius) {
	adapter::MedianRadiusAdapter<T>	mra(image, radius);
	return ImagePtr(new Image<T>(mra));
}

#define destar_mono(Pixel, imageptr)					\
{									\
	Image<Pixel>	*image						\
		= dynamic_cast<Image<Pixel>*>(&*imageptr);		\
	if (NULL != image) {						\
		return destar(*image, radius);				\
	}								\
}

#define destar_color(Pixel, imageptr)					\
{									\
	Image<RGB<Pixel> >	*image					\
		= dynamic_cast<Image<RGB<Pixel> >*>(&*imageptr);	\
	if (NULL != image) {						\
		adapter::LuminanceAdapter<RGB<Pixel>, float>	la(*image);	\
		Image<float>	*limage = new Image<float>(la);		\
		ImagePtr	limageptr(limage);			\
		return destar(*limage, radius);				\
	}								\
}

ImagePtr	do_destar(ImagePtr imageptr, int radius) {
	destar_mono(unsigned char, imageptr)
	destar_mono(unsigned short, imageptr)
	destar_mono(unsigned int, imageptr)
	destar_mono(unsigned long, imageptr)
	destar_mono(float, imageptr)
	destar_mono(double, imageptr)
	destar_color(unsigned char, imageptr)
	destar_color(unsigned short, imageptr)
	destar_color(unsigned int, imageptr)
	destar_color(unsigned long, imageptr)
	destar_color(float, imageptr)
	destar_color(double, imageptr)
	throw std::runtime_error("unknown pixel type");
}

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	force = false;
	double	radius = 1;
	while (EOF != (c = getopt_long(argc, argv, "dh?fr:", longopts,
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
		case 'r':
			radius = std::stod(optarg);
			break;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// make sure we have at least 2 files
	if (optind >= argc) {
		std::cerr << "must specify image to get destar" << std::endl;
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

	// destar masking of image
	ImagePtr	outimage = do_destar(image, radius);

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

} // namespace destar
} // namespace app
} // namespace astro

int     main(int argc, char *argv[]) {
        return astro::main_function<astro::app::destar::main>(argc, argv);
}

