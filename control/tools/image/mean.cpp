/*
 * mean.cpp -- compute the mean of the images
 *
 * (c) 2026 Prof Dr Andreas Müller
 */
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <getopt.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroImage.h>

namespace astro {
namespace app {
namespace mean {

/**
 * \brief Write a help message
 *
 * \param progname 	name under which the program was called
 */
static void	usage(const char *progname) {
	std::cout << "usage:" << std::endl;
	std::cout << "   " << progname << " [ options ] files" << std::endl;
	std::cout << "options:" << std::endl;
}

/**
 * \brief Options for the fold program
 */
static struct option    longopts[] = {
{ "debug",      no_argument,            NULL,           'd' },
{ "help",	no_argument,		NULL,		'h' },
{ "number",	required_argument,	NULL,		'n' },
{ "outfile",	required_argument,	NULL,		'o' },
{ NULL,		0,			NULL,		 0  }
};

#define addimage(imageptr, pixel)					\
{									\
	image::Image<pixel>	*summand				\
		= dynamic_cast<image::Image<pixel>*>(&*imageptr);	\
	if (NULL != summand) {						\
		img->add(*summand);					\
	}								\
}

/**
 * \brief Main function for the mean program
 *
 * \param argc	number of arguments
 * \param argv	parameter array
 */
static int	main(int argc, char *argv[]) {
	std::string	outfilename;
	int	number = -1; // unlimited number of images

	// parse command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?o:n:",
			longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'n':
			number = std::stoi(optarg);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "limit to %d images",
				number);
			break;
		case 'o':
			outfilename = std::string(optarg);
			break;
		}

	// read all the input files
	int	counter = 0;
	image::Image<image::RGB<float>>	*img = NULL;
	image::ImagePtr	result;
	while ((optind < argc) && ((number < 0) || (counter < number))) {
		std::string	infilename(argv[optind++]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing file %s",
			infilename.c_str());
		image::ImagePtr	newimage;
		// read the image
		if (image::PNG::ispngfilename(infilename)) {
			image::PNG	png;
			newimage = png.readPNG(infilename);
		} else if (image::JPEG::isjpegfilename(infilename)) {
			image::JPEG	jpeg;
			newimage = jpeg.readJPEG(infilename);
		} else {
			std::string	msg = stringprintf("don't know how to "
				"process file %s", infilename.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		counter++;
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d: found a %s-image to process",
			counter, newimage->info().c_str());
		// if this is the first image, create the accumulator image
		if (!img) {
			img = new image::Image<image::RGB<float>>(newimage->size());
			result = image::ImagePtr(img);
		}
		// accumulate image
		addimage(newimage, image::RGB<unsigned char>)
	}
	// rescale with the scale factor
	image::Image<image::RGB<unsigned char>>	*meanimage
		= new image::Image<image::RGB<unsigned char>>(*img,
			1. / counter);
	image::ImagePtr	outimage(meanimage);
		
	// write output file as a JPEG image
	if (image::PNG::ispngfilename(outfilename)) {
		image::PNG	png;
		png.writePNG(outimage, outfilename);
	} else if(image::JPEG::isjpegfilename(outfilename)) {
		image::JPEG	jpeg;
		jpeg.writeJPEG(outimage, outfilename);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no outfilename");
	}

	return EXIT_SUCCESS;
}

} // namespace mean
} // namespace app
} // app

int	main(int argc, char *argv[]) {
	try {
		return astro::app::mean::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "mean programm terminated by exception "
			<< x.what() << std::endl;
	} catch (...) {
		std::cerr << "mean programm terminated by unknown exception"
			<< std::endl;
	}
	return EXIT_FAILURE;
}
