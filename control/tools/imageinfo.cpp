/*
 * imageinfo.cpp -- display information about images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroFilter.h>
#include <debug.h>
#include <stdexcept>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::io;

namespace astro {

/**
 * \brief Show image info
 */
void	show_imageinfo(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image info for: %s", filename.c_str());
	try {
		std::cout << "name: " << filename << std::endl;
		FITSin	infile(filename);
		ImagePtr	image = infile.read();
		std::cout << "size: " << image->size.width << " x "
			<< image->size.height <<std::endl;

		// find maximum, minimum, average and median values
		double	maximum = astro::image::filter::max(image);
		double	minimum = astro::image::filter::min(image);
		double	mean = astro::image::filter::mean(image);
		double	median = astro::image::filter::median(image);
		std::cout << "min = " << minimum;
		if (median < mean) {
			std::cout << ", median = " << median;
			std::cout << ", mean = " << mean;
		} else {
			std::cout << ", mean = " << mean;
			std::cout << ", median = " << median;
		}
		std::cout << ", max = " << maximum;
		std::cout << std::endl;
	} catch (std::exception& x) {
		std::cerr << "could not process " << filename << ": "
			<< x.what() << std::endl;
	}
}

/**
 * \brief main function
 */
int	main(int argc, char *argv[]) {
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	if (argc <= optind) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no image file arguments");
		throw std::runtime_error("not image file arguments");
	}

	int	counter = 0;
	for (; optind < argc; optind++) {
		if (counter++ > 0) {
			std::cout << std::endl;
		}
		std::string	filename(argv[optind]);
		show_imageinfo(filename);
	}
	
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "imageinfo terminated: %s",
			x.what());
	}
}
