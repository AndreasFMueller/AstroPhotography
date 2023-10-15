/*
 * haar.cpp -- haar wavelet transform application
 *
 * (c) 2023 Prof Dr Andreas Müller
 */
#include <includes.h>
#include <AstroImage.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <AstroWavelets.h>
#include <AstroDebug.h>

namespace astro {
namespace app {
namespace haar {

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] file "
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Perform a Haar Wavelet transform on the image"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug           increase debug level" << std::endl;
	std::cout << " -f,--force           overwrite outputfile if present"
		<< std::endl;
	std::cout << " -h,-?,--help         display this help" << std::endl;
	std::cout << " -i,--inverse         perform inverse transform" << std::endl;
}

static struct option    longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "inverse",	no_argument,		NULL,		'i' }, /* 3 */
{ NULL,		0,			NULL,		 0  }
};

#define do_transform(inimage, pixelin, pixelout)			\
	{								\
		Image<pixelin >	*imagep					\
			= dynamic_cast<Image<pixelin >*>(&*inimage);	\
		if (imagep != NULL) {					\
			return ImagePtr(new Image<pixelout >(*imagep));	\
		}							\
	}
	
ImagePtr	convert_to_float(ImagePtr inimage) {
	do_transform(inimage, unsigned char, float)
	do_transform(inimage, unsigned short, float)
	do_transform(inimage, unsigned int, float)
	do_transform(inimage, unsigned long, float)
	do_transform(inimage, float, float)
	do_transform(inimage, double, double)
	do_transform(inimage, RGB<unsigned char>, RGB<float>)
	do_transform(inimage, RGB<unsigned short>, RGB<float>)
	do_transform(inimage, RGB<unsigned int>, RGB<float>)
	do_transform(inimage, RGB<unsigned long>, RGB<float>)
	do_transform(inimage, RGB<float>, RGB<float>)
	do_transform(inimage, RGB<double>, RGB<double>)
	throw std::runtime_error("unknown pixel type");
}

int	main(int argc, char *argv[]) {
	bool	inverse = false;
	bool	force = false;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?n:s:", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			inverse = true;
			break;
		case 'f':
			force = true;
			break;
		default:
			std::cerr << "unknown option: " << c << std::endl;
			break;
	}

	// get the filename from the command line
	if (argc <= optind) {
		std::cerr << "input image file argument missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	inputfilename(argv[optind++]);
	if (argc <= optind) {
		std::cerr << "output image file argument missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outputfilename(argv[optind++]);

	// read the input image
	io::FITSin	in(inputfilename);
	ImagePtr	inimage = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "input image: %s",
		inimage->info().c_str());

	// convert the image to 
	ImagePtr	inputimage = convert_to_float(inimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converted to: %s",
		inputimage->info().c_str());

	ImagePtr	outimage = adapter::haarwavelettransform(inputimage,
				inverse);
	io::FITSout	out(outputfilename);
	if (force) {
		out.setPrecious(false);
	}
	out.write(outimage);

	return EXIT_SUCCESS;
}

} // namespace haar
} // namespace app
} // namespace astro


int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::haar::main>(argc, argv);
}

