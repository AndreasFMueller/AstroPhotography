/*
 * transform.cpp -- some image transformations
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroTransform.h>
#include <AstroAdapter.h>
#include <AstroIO.h>
#include <cstdlib>
#include <cstdio>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>

using namespace astro;
using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace transform {

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] <in.fits> "
		"<out.fits>" << std::endl;
	std::cout << std::endl;
	std::cout << "read an image from <in.fits>, translate or rotate it, "
		"and write the result" << std::endl;
	std::cout << "to <out.fits>." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug          increase debug level"
		<< std::endl;
	std::cout << "    -a,-angle=<angle>   rotate through angle <angle>"
		<< std::endl;
	std::cout << "    -x,--x-offset=<x>   translate <x> in x-direction"
		<< std::endl;
	std::cout << "    -y,--y-offset=<y>   translate <y> in y-direction"
		<< std::endl;
	std::cout << "    -s,--sample=<value> down or upsample the image"
		<< std::endl;
	std::cout << "    -h,-?,--help        display this help message and "
		"exit" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,	'd' }, /* 0 */
{ "angle",	required_argument,	NULL,	'a' }, /* 0 */
{ "x-offset",	required_argument,	NULL,	'x' }, /* 0 */
{ "y-offset",	required_argument,	NULL,	'y' }, /* 0 */
{ "sample",	required_argument,	NULL,	's' }, /* 0 */
{ "help",	no_argument,		NULL,	'h' }, /* 0 */
};

int	main(int argc, char *argv[]) {
	// parse command line
	int	c;
	int	longindex;
	Point	translation;
	int	sample = 0;
	double	angle = 0;
	while (EOF != (c = getopt_long(argc, argv, "dx:y:s:a:h?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'a':
			angle = atof(optarg);
			break;
		case 'x':
			translation.setX(atof(optarg));
			break;
		case 'y':
			translation.setY(atof(optarg));
			break;
		case 's':
			sample = atoi(optarg);
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}

	// next two arguments must be filenames
	if ((argc - optind) != 2) {
		std::cerr << "need exactly two file name arguments"
			<< std::endl;
		return EXIT_FAILURE;
	}
	const char	*infilename = argv[optind++];
	const char	*outfilename = argv[optind++];

	// read the image from the file
	FITSin	infile(infilename);
	ImagePtr	image = infile.read();

	// prepare result
	ImagePtr	result;
	if (0 != angle) {
		// perform rotation
		Transform	rotation(angle, translation);
		result = astro::image::transform::transform(image, rotation);
	} else {
		// apply a sampling adapter and a translation adapter
		if (sample > 0) {
			ImageSize	sampling(1 + sample, 1 + sample);
			result = translate(upsample(image, sampling), translation);
		}
		if (sample < 0) {
			ImageSize	sampling(1 - sample, 1 - sample);
			result = translate(downsample(image, sampling), translation);
		}
		if (sample == 0) {
			result = translate(image, translation);
		}
	}
	
	// write the result image
	unlink(outfilename);
	FITSout	outfile(outfilename);
	outfile.write(result);

	// that's it
	return EXIT_SUCCESS;
}

} // namespace transform
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::transform::main>(argc, argv);
}
