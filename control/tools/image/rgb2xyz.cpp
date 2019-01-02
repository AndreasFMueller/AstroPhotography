/*
 * rgb2xyz.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroImage.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroAdapter.h>

namespace astro {
namespace app {
namespace rgb2xyz {

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ otions ] from.fits to.fits";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "convert and RGB image into an XYZ image";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug     increase debug level" << std::endl;
	std::cout << "  -h,-?,--help   display this help message and exit";
	std::cout << std::endl;
	std::cout << "  -r,--reverse   convert in the reverse direction, i.e. from XYZ to RGB";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,	'd' },
{ "help",	no_argument,		NULL,	'h' },
{ "reverse",	no_argument,		NULL,	'r' },
{ NULL,		0,			NULL,	 0  }
};

#define do_convert(P, Q, image)						\
{									\
	Image<P >	*from = dynamic_cast<Image<P >*>(&*image);	\
	if (NULL != from) {						\
		adapter::ColorConversionAdapter<Q, P >	cc(*from);	\
		Image<Q >	*to = new Image<Q >(cc);		\
		ImagePtr	converted(to);				\
		return converted;					\
	}								\
}

ImagePtr	convertXYZtoRGB(ImagePtr image) {
	do_convert(XYZ<unsigned char>, RGB<float>, image)
	do_convert(XYZ<unsigned short>, RGB<float>, image)
	do_convert(XYZ<unsigned int>, RGB<float>, image)
	do_convert(XYZ<unsigned long>, RGB<float>, image)
	do_convert(XYZ<float>, RGB<float>, image)
	do_convert(XYZ<double>, RGB<double>, image)
	return ImagePtr();
}

ImagePtr	convertRGBtoXYZ(ImagePtr image) {
	do_convert(RGB<unsigned char>, XYZ<float>, image)
	do_convert(RGB<unsigned short>, XYZ<float>, image)
	do_convert(RGB<unsigned int>, XYZ<double>, image)
	do_convert(RGB<unsigned long>, XYZ<double>, image)
	do_convert(RGB<float>, XYZ<float>, image)
	do_convert(RGB<double>, XYZ<double>, image)
	return ImagePtr();
}

ImagePtr	convert(ImagePtr image, bool reverse) {
	return (reverse) ? convertXYZtoRGB(image) : convertRGBtoXYZ(image);
}

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	reverse = false;
	while (EOF != (c = getopt_long(argc, argv, "dh?r", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'r':
			reverse = true;
			break;
		}

	// get two file names
	if ((optind + 2) > argc) {
		std::cerr << "not enough arguments" << std::endl;
		return EXIT_FAILURE;
	}

	std::string	infile(argv[optind++]);
	std::string	outfile(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting %s(%s) to %s(%s)",
		infile.c_str(), (reverse) ? "XYZ" : "RGB",
		outfile.c_str(), (reverse) ? "RGB" : "XYZ");

	// reading the input file
	io::FITSin	in(infile);
	ImagePtr	image = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "input Image<%s> %s",
		demangle(image->pixel_type().name()).c_str(),
		image->size().toString().c_str());

	// convert the image
	ImagePtr	outimage = convert(image, reverse);
	if (!outimage) {
		std::cerr << "cannot convert" << std::endl;
		return EXIT_FAILURE;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "output Image<%s> %s",
		demangle(outimage->pixel_type().name()).c_str(),
		outimage->size().toString().c_str());

	io::FITSout	out(outfile);
	out.setPrecious(false);
	out.write(outimage);

	return EXIT_SUCCESS;
}

} // namespace rgbxyz
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::rgb2xyz::main>(argc, argv);
}
