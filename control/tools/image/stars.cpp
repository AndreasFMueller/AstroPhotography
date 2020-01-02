/*
 * stars.cpp -- extract stars from an image
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroImage.h>
#include <AstroTransform.h>
#include <AstroIO.h>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::image::transform;

namespace astro {
namespace app {
namespace stars {

void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] file "
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Find stars in an image and display there coordinates and "
		"brightness." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug           increase debug level" << std::endl;
	std::cout << " -h,-?,--help         display this help" << std::endl;
	std::cout << " -n,--number=<n>      number of stars to extract"
		<< std::endl;
	std::cout << " -r,--radius=<r>      search radius for star extraction"
		<< std::endl;
}

static struct option    longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "help",	no_argument,		NULL,		'h' }, /* 1 */
{ "number",	required_argument,	NULL,		'n' }, /* 1 */
{ "radius",	required_argument,	NULL,		'r' }, /* 1 */
{ NULL,		0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	int	numberofstars = 10;
	int	searchradius = 10;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?n:s:", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'n':
			numberofstars = std::stoi(optarg);
			break;
		case 's':
			searchradius = std::stoi(optarg);
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}	

	// get the filename from the command line
	if (argc <= optind) {
		std::cerr << "image file argument missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	filename(argv[optind++]);

	// read the file
	io::FITSin	in(filename);
	ImagePtr	image = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image size: %s",
		image->size().toString().c_str());

	// find the stars
	StarExtractor	starextractor(numberofstars, searchradius);
	adapter::LuminanceExtractor	luminance(image);
	StarAcceptanceCriterion	criterion(luminance);
	std::vector<Star>	stars = starextractor.stars(image, criterion);
	for (auto ptr = stars.begin(); ptr != stars.end(); ptr++) {
		std::cout << ptr->toString() << std::endl;
	}

	return EXIT_SUCCESS;
}

} // namespace stars
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::stars::main>(argc, argv);
}
