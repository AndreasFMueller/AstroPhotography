/*
 * findtransform.cpp -- find the transform between two images
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
namespace findtransform {

void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] <from> <to> "
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Find transform that transforms <from> image into <to> image" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug           increase debug level" << std::endl;
	std::cout << " -h,-?,--help         display this help" << std::endl;
	std::cout << " -n,--number=<n>      number of stars to use"
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
	int	numberofstars = 20;
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
		}	

	// get the filename from the command line
	if (argc <= optind) {
		std::cerr << "from image file argument missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	fromfilename(argv[optind++]);
	if (argc <= optind) {
		std::cerr << "to image file argument missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	tofilename(argv[optind++]);

	// read the file
	io::FITSin	fromin(fromfilename);
	ImagePtr	fromimage = fromin.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "from image size: %s",
	fromimage->size().toString().c_str());

	io::FITSin	toin(tofilename);
	ImagePtr	toimage = toin.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "to image size: %s",
	toimage->size().toString().c_str());

	// find the transform
#if 0
	TriangleSetFactory	factory;
	factory.numberofstars(numberofstars);
	factory.radius(searchradius);
	TriangleSet	fromtriangles = factory.get(fromimage);
	TriangleSet	totriangles = factory.get(toimage);
	Transform	transform = fromtriangles.closest(totriangles);
#else
	TriangleAnalyzer	analyzer(fromimage, numberofstars, searchradius);
	Transform	transform = analyzer.transform(toimage);
#endif

	std::cout << "Transform found: " << transform.toString() << std::endl;

	return EXIT_SUCCESS;
}

} // namespace findtransform
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::findtransform::main>(argc, argv);
}

