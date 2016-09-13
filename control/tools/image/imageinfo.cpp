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
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroProject.h>

using namespace astro;
using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::project;
using namespace astro::config;
using namespace astro::io;

namespace astro {
namespace app {
namespace imageinfo {

static std::string	reponame;

bool	verbose = false;
bool	fitsinfo = false;

/**
 * \brief Common image info display function
 */
int	show_imageinfo(ImagePtr image) {
	if (fitsinfo) {
		std::cout << *image;
	}

	if (verbose) {
		std::cout << "bpp=" << image->bitsPerPixel();
		std::cout << " Bpp=" << image->bytesPerPixel();
		std::cout << " planes=" << image->planes();
		std::cout << " minvalue=" << image->minimum();
		std::cout << " maxvalue=" << image->maximum();
		std::cout << " pixel_type=";
		std::cout << demangle(image->pixel_type().name());
		std::cout << " ";
	}

	// find maximum, minimum, average and median values
	double	maximum = astro::image::filter::max(image);
	double	minimum = astro::image::filter::min(image);
	double	mean = astro::image::filter::mean(image);
	double	median = astro::image::filter::median(image);
	double	nans = -1;
	try {
		nans = astro::image::filter::countnans(image);
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring nans");
	}
	
	std::cout << "min=" << minimum;
	if (median < mean) {
		std::cout << " median=" << median;
		std::cout << " mean=" << mean;
	} else {
		std::cout << " mean=" << mean;
		std::cout << " median=" << median;
	}
	std::cout << " max=" << maximum;
	if (nans >= 0) {
		std::cout << " nans=" << nans;
	}
	if (image->getMosaicType() != MosaicType()) {
		RGB<double>	meancolor
			= astro::image::filter::mean_color(image);
		std::cout << " red=" << meancolor.R;
		std::cout << " green=" << meancolor.G;
		std::cout << " blue=" << meancolor.B;
	}

	return EXIT_SUCCESS;
}

/**
 * \brief Show image info for an image file
 */
int	show_imagefile(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image info for: %s", filename.c_str());
	try {
		std::cout << filename << ": ";
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
		ConfigurationPtr	config = Configuration::get();
		ImageRepoConfigurationPtr	imagerepos
			= ImageRepoConfiguration::get(config);
		ImageRepoPtr	repo = imagerepos->repo(reponame);
		long	imageid = std::stol(argument);
		std::cout << imageid << ": ";
		ImagePtr	image = repo->getImage(imageid);
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
static void	usage(const std::string& progname) {
	std::cout << "Usage:" << std::endl;
	Path	path(progname);
	std::cout << path.basename() << " [ options ] images ..." << std::endl;
	std::cout << "Display information about pixel values of FITS images "
		"specified by their" << std::endl;
	std::cout << "file names in the images arguments." << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -c,--config=cfg    use configuration database <cfg> "
		"instead of" << std::endl;
	std::cout << "                     default ~/astrophoto.db";
	std::cout << std::endl;
	std::cout << "  -d,--debug         increase debug level" << std::endl;
	std::cout << "  -f,--fits          display FITS header info" << std::endl;
	std::cout << "  -h,--help          display help message end exit";
	std::cout << std::endl;
	std::cout << "  -r,--repo=<repo>   use image repository named <repo> instead of direct" << std::endl;
	std::cout << "                     file access. In this case, the images arguments are" << std::endl;
	std::cout << "                     numbers (ids) of images in the repository, not file" << std::endl;
	std::cout << "                     names" << std::endl;;
	std::cout << "  -v,--verbose       verbose mode, show additional "
		"information about" << std::endl;
	std::cout << "                     pixel types" << std::endl;
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
{ "verbose",	no_argument,		NULL,		'v' }, /* 4 */
{ "fits",	no_argument,		NULL,		'f' }, /* 5 */
{ NULL,         0,			NULL,		0   }
};

/**
 * \brief main function
 */
int	main(int argc, char *argv[]) {
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
		case 'f':
			fitsinfo = true;
			break;
		case 'h':
			usage(std::string(argv[0]));
			return EXIT_FAILURE;
		case 'r':
			reponame = optarg;
			break;
		case 'v':
			verbose = true;
			break;
		}

	if (argc <= optind) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no image file arguments");
		throw std::runtime_error("not image file arguments");
	}

	for (; optind < argc; optind++) {
		std::string	argument(argv[optind]);
		if (reponame.size() != 0) {
			show_imagerepo(argument);
		} else {
			show_imagefile(argument);
		}
		std::cout << std::endl;
	}

	return EXIT_SUCCESS;
}

} // namespace imageinfo
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::imageinfo::main>(argc, argv);
}
