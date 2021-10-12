/*
 * psf.cpp -- point spread extraction program
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroIO.h>
#include <AstroPsf.h>
#include <AstroAdapter.h>

using namespace astro::image;
using namespace astro::io;
using namespace astro::psf;

namespace astro {
namespace app {
namespace psf {

static void     usage(const char *progname) {
	Path    p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] <in.fits> "
		"<out.fits>" << std::endl;
	std::cout << std::endl;
	std::cout << "read an image from <in.fits>, extract a point spread "
		"function, and write the" << std::endl;
	std::cout << "result to <out.fits>." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug          increase debug level"
		<< std::endl;
	std::cout << "    -r,--radius=<r>      radius of the psf (default 32)"
		<< std::endl;
	std::cout << "    -s,--stars=<s>      number of stars to use for the "
		"psf (default 10)" << std::endl;
	std::cout << "    -h,-?,--help        display this help message and "
		"exit" << std::endl;
	std::cout << std::endl;
}

static struct option    longopts[] = {
{ "crop",	no_argument,		NULL,	'c' }, /* 0 */
{ "debug",	no_argument,		NULL,	'd' }, /* 1 */
{ "radius",	required_argument,	NULL,	'r' }, /* 2 */
{ "stars",	required_argument,	NULL,	's' }, /* 3 */
{ "help",	no_argument,		NULL,	'h' }, /* 4 */
{ NULL,		0,			NULL,	 0  }
};

int     main(int argc, char *argv[]) {
	// parse command line
	int	c;
	int	longindex;
	int	stars = 10;
	int	radius = 32;
	bool	crop = false;
	while (EOF != (c = getopt_long(argc, argv, "dr:s:h?S",
		longopts, &longindex)))
		switch (c) {
		case 'c':
			crop = true;
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'r':
			radius = std::stoi(optarg);
			break;
		case 's':
			stars = std::stoi(optarg);
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
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

	// build the PsfExtractor
	PsfExtractor	psfextractor;
	psfextractor.radius(radius);
	psfextractor.maxstars(stars);

	// extract the psf from the input image
	Image<double>	*psf = psfextractor.extract(image);
	if (NULL == psf) {
		std::string	msg = stringprintf("PSF extraction failed");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// write the image to the output file
	FITSoutfile<double>	outfile(outfilename);
	outfile.setPrecious(false);
	if (crop) {
		ImagePoint	center = psf->getFrame().center();
		ImageRectangle	rectangle(center - ImagePoint(radius, radius),
					ImageSize(2 * radius, 2 * radius));
		adapter::WindowAdapter<double>	window(*psf, rectangle);
		Image<double>	*cropped = new Image<double>(window);
		outfile.write(*cropped);
		delete cropped;
	} else {
		outfile.write(*psf);
	}

	// cleanup 
	delete psf;

	return EXIT_SUCCESS;
}

} // namespace psf
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::psf::main>(argc, argv);
}
