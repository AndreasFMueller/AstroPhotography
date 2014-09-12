/*
 * imageinfo.cpp -- display information about images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <stacktrace.h>
#include <AstroConfig.h>
#include <AstroProject.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::project;
using namespace astro::config;
using namespace astro::io;

namespace astro {

static std::string	reponame;

/**
 * \brief Common image info display function
 */
int	show_imageinfo(ImagePtr image) {
	std::cout << *image;

	std::cout << "bits per pixel:  " << image->bitsPerPixel() << std::endl;
	std::cout << "bytes per pixel: " << image->bytesPerPixel() << std::endl;
	std::cout << "planes:          " << image->planes() << std::endl;
	std::cout << "minimum value:   " << image->minimum() << std::endl;
	std::cout << "maximum value:   " << image->maximum() << std::endl;
	std::cout << "pixel type:      " << image->pixel_type().name();
	std::cout << std::endl;

	// find maximum, minimum, average and median values
	double	maximum = astro::image::filter::max(image);
	double	minimum = astro::image::filter::min(image);
	double	mean = astro::image::filter::mean(image);
	double	median = astro::image::filter::median(image);
	double	nans = astro::image::filter::countnans(image);
	
	std::cout << "min = " << minimum;
	if (median < mean) {
		std::cout << ", median = " << median;
		std::cout << ", mean = " << mean;
	} else {
		std::cout << ", mean = " << mean;
		std::cout << ", median = " << median;
	}
	std::cout << ", max = " << maximum;
	std::cout << ", nans = " << nans;
	std::cout << std::endl;

	return EXIT_SUCCESS;
}

/**
 * \brief Show image info for an image file
 */
int	show_imagefile(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image info for: %s", filename.c_str());
	try {
		std::cout << "name: " << filename << std::endl;
		FITSin	infile(filename);
		ImagePtr	image = infile.read();

		return show_imageinfo(image);
	} catch (std::exception& x) {
		std::cerr << "could not process " << filename << ": ";
		std::cerr << x.what() << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief Display information about an image found in the repository
 */
int	show_imagerepo(const std::string& argument) {
	try {
		ImageRepo	repo = Configuration::get()->repo(reponame);
		ImagePtr	image = repo.getImage(std::stol(argument));
		return show_imageinfo(image);
	} catch (std::exception& x) {
		std::cerr << "could not process " << argument;
		std::cerr << " in repo " << reponame << ": " << x.what();
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief usage function for the imageinfo program
 */
void	usage(const std::string& progname) {
}

/**
 * \brief Table of options
 */
static struct option    longopts[] = {
/* name         argument?               int*            int */
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "repo",	required_argument,	NULL,		'r' }, /* 3 */
{ NULL,         0,			NULL,		0   }
};

/**
 * \brief main function
 */
int	imageinfo_main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dr:", longopts,
		&longindex)))
		switch (c) {
		case 'c':
			Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(std::string(argv[0]));
			return EXIT_FAILURE;
		case 'r':
			reponame = optarg;
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
		std::string	argument(argv[optind]);
		if (reponame.size() != 0) {
			show_imagerepo(argument);
		} else {
			return show_imagefile(argument);
		}
	}
	
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return astro::imageinfo_main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by " << typeid(x).name() << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
