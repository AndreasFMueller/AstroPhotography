/*
 * deconvolve.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <includes.h>
#include <AstroConvolve.h>

namespace astro {
namespace app {
namespace deconvolve {

/**
 * \brief Show usage of programm
 *
 * \param progname	name of the program
 */
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
	std::cout << "    -h,-?,--help        display this help message and "
		"exit" << std::endl;
	std::cout << "    -p,--psf=<file>     point spread function file"
		<< std::endl;
	std::cout << "    -m,--method=<meth>  method either 'vancittert' or 'fourier'"
		<< std::endl;
	std::cout << "    -i,--iterations=<n> number of iterations in vancittert"
		<< std::endl;
}

static struct option    longopts[] = {
{ "debug",		no_argument,		NULL,	'd' }, /* 0 */
{ "help",		no_argument,		NULL,	'h' }, /* 1 */
{ "psf",		required_argument,	NULL,	'p' }, /* 2 */
{ "method",		required_argument,	NULL,	'm' }, /* 3 */
{ "iterations",		required_argument,	NULL,	'i' }, /* 4 */
{ NULL,			0,			NULL,	 0  }
};


/**
 * \brief main function of the deconvolve program
 * 
 * \param argc	the number of arcuments
 * \param argv	the arguments themselves
 */
int	main(int argc, char *argv[]) {
	// parse command line
	int	c;
	int	longindex;
	int	iterations = 10;
	ImagePtr	psf;
	std::string	method("vancittert");
	while (EOF != (c = getopt_long(argc, argv, "dh?pm:i:",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'p':
			{
			io::FITSin	psffile(optarg);
			psf = psffile.read();
			}
			break;
		case 'm':
			method = std::string(optarg);
			break;
		case 'i':
			iterations = std::stoi(optarg);
			break;
		}

	// make sure we have a psf
	if (!psf) {
		std::cerr << "there is no PSF defined" << std::endl;
		return EXIT_FAILURE;
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
	io::FITSin	infile(infilename);
	ImagePtr	image = infile.read();

	// perform the deconvolution
	ImagePtr	outimage(NULL);
	if (method == std::string("fourier")) {
		throw std::runtime_error("fourier not implemented");
	}
	if (method == std::string("vancittert")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "performing vancittert deconvolution");
		VanCittertOperator	vc(psf);
		vc.iterations(iterations);
		outimage = vc(image);
	}
	if (!outimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown method '%s'", method.c_str());
	}
	io::FITSout	outfile(outfilename);
	outfile.setPrecious(false);
	outfile.write(outimage);
	return EXIT_SUCCESS;
}

} // namespace deconvolve
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main_function<astro::app::deconvolve::main>(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "cannot deconvolve: " << x.what() << std::endl;
		return EXIT_FAILURE;
	}
}

