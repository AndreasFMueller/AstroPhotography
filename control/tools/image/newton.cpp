/*
 * newton.cpp -- 
 *
 * (c) 2026 Prof Dr Andreas Müller
 */
#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroTypes.h>
#include <getopt.h>
#include <complex>

namespace astro {
namespace app {
namespace newton {

/**
 * \brief display a help message
 *
 * \param progname	name under which the program was called
 */
static void	usage(char *progname) {
	std::cout << "usage:" << std::endl;
	std::cout << "    " << progname << " [ options ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -c,--center=<c>       image center" << std::endl;
	std::cout << " -d,--debug            turn on debugging" << std::endl;
	std::cout << " -h,--help,-?          show a help message and terminate"
		<< std::endl;
	std::cout << " -o,--outfile=<o>      output file name"
		<< std::endl;
	std::cout << " -r,--resolution=<r>   image resolution (pixel width)"
		<< std::endl;
}

/**
 * \brief long options for the newton program
 */
static struct option	longopts[] = {
{ "center",	required_argument,	NULL,		'c' },
{ "debug",      no_argument,            NULL,           'd' },
{ "graded",	no_argument,		NULL,		'g' },
{ "help",	no_argument,		NULL,		'h' },
{ "outfile",	required_argument,	NULL,		'o' },
{ "resolution",	required_argument,	NULL,		'r' },
{ "size",	required_argument,	NULL,		's' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief 
 *
 * \param 
 * \return color to use for this pixel
 */
static image::RGB<unsigned char>	iteration(const Point& p, bool graded) {
	std::complex<double>	z(p.x(), p.y());
	int	counter = 0;
	const static int	N = 20;
	while (counter < N) {
		double	lambda = (graded) ? 1 - counter / (double)N : 1.;
		counter++;
		z = (2.*z*z*z+1.)/(3.*z*z);
		const static double	epsilon = 1e-4;
		const static std::complex<double>	z1(1);
		if (abs(z - z1) < epsilon) {
			return image::RGB<unsigned char>(
				(unsigned char)204,
				(unsigned char)102,
				(unsigned char)102) * lambda;
		}
		const static std::complex<double>	z2(-0.5, sqrt(3)/2);
		if (abs(z - z2) < epsilon) {
			return image::RGB<unsigned char>(
				(unsigned char)102,
				(unsigned char)154,
				(unsigned char)102) * lambda;
		}
		const static std::complex<double>	z3(-0.5, -sqrt(3)/2);
		if (abs(z - z3) < epsilon) {
			return image::RGB<unsigned char>(
				(unsigned char)102,
				(unsigned char)102,
				(unsigned char)255) * lambda;
		}
	}
	return image::RGB<unsigned char>(0);
}

/**
 * \brief main function for the newton program
 *
 * \param argc	number of arguments
 * \param argv	argument vector
 */
static int	main(int argc, char *argv[]) {
	// static variables 
	Point	center;
	image::ImageSize	size(1001, 1001);
	double	resolution = 0.01;
	std::string	outputfilename;
	bool	graded = false;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dgh?o:r:s:", longopts,
		&longindex)))
		switch (c) {
		case 'c':
			center = Point(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'g':
			graded = true;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			break;
		case 'o':
			outputfilename = std::string(optarg);
			break;
		case 'r':
			resolution = std::stod(optarg);
			break;
		case 's':
			size = ImageSize(optarg);
			break;
		}

	// prepare the image
	image::Image<image::RGB<unsigned char>>	*img
		= new image::Image<image::RGB<unsigned char>>(size);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size %s image created",
		img->size().toString().c_str());
	ImagePtr	imgptr(img);

	// perform the iteration
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start iteration");
	double	xmin = center.x() - resolution * size.width() / 2;
	double	ymin = center.y() - resolution * size.height() / 2;
	for (int xi = 0; xi < size.width(); xi++) {
		double	x = xmin + xi * resolution;
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %f\n", x);
		for (int yi = 0; yi < size.height(); yi++) {
			double	y = ymin + yi * resolution;
			Point	p(x, y);
			img->writablepixel(xi, yi) = iteration(p, graded);
		}
	}
	
	// write the image to a file
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing image file %s",
		outputfilename.c_str());
	if (image::JPEG::isjpegfilename(outputfilename)) {
		image::JPEG	jpeg;
		jpeg.writeJPEG(imgptr, outputfilename);
	} else if (image::PNG::ispngfilename(outputfilename)) {
		image::PNG	png;
		png.writePNG(imgptr, outputfilename);
	} else {
		std::string	msg = stringprintf("don't know how to write "
			"file '%s'", outputfilename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		return EXIT_FAILURE;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image file written");

	return EXIT_SUCCESS;
}

} // namespace newton
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::app::newton::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "newton terminated by exception " << x.what()
			<< std::endl;
	} catch (...) {
		std::cerr << "newton terminated by unknown exception "
			<< std::endl;
	}
	return EXIT_FAILURE;
}
