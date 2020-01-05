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
{ "constrained",	no_argument,		NULL,	'c' }, /* 0 */
{ "debug",		no_argument,		NULL,	'd' }, /* 1 */
{ "epsilon", 		required_argument,	NULL,	'e' }, /* 2 */
{ "gauss",		no_argument,		NULL,	'g' }, /* 2 */
{ "help",		no_argument,		NULL,	'h' }, /* 3 */
{ "psf",		required_argument,	NULL,	'p' }, /* 4 */
{ "prefix",		required_argument,	NULL,	'P' }, /* 5 */
{ "stddev",		required_argument,	NULL,	's' }, /* 6 */
{ "method",		required_argument,	NULL,	'm' }, /* 7 */
{ "iterations",		required_argument,	NULL,	'i' }, /* 8 */
{ "k",			required_argument,	NULL,	'k' },
{ NULL,			0,			NULL,	 0  }
};

/**
 * \brief Find the center point of an image
 *
 * \param psf	the image to analyze
 */
static Point	findcenter(const ConstImageAdapter<double>& psf) {
	double	xs = 0;
	double	ys = 0;
	double	t = 0;
	int	w = psf.getSize().width();
	int	h = psf.getSize().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	m = psf.pixel(x, y);
			t += m;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel(%d,%d) = %g", x, y, m);
			xs += m * x;
			ys += m * y;
		}
	}
	return Point(xs / t, ys / t);
}

/**
 * \brief Find the standard deviation of the image
 *
 * \param psf	the image to analyze
 */
static double	findstddev(const ConstImageAdapter<double>& psf) {
	Point	center = findcenter(psf);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "center: %s",
		center.toString().c_str());
	int	counter = 0;
	int	w = psf.getSize().width();
	int	h = psf.getSize().height();
	double	s = 0;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	d = (center - Point(x, y)).abs();
			s += d * d;
			counter++;
		}
	}
	double	stddev = sqrt(s / counter);
	stddev /= sqrt(2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stddev found: %f", stddev);
	return stddev;
}

/**
 * \brief Construct a gaussian image with a given standard deviation
 *
 * \param stddev	the standard deviation to use for the gaussian
 */
static ImagePtr	gausspsf(float stddev) {
	Image<double>	*psf = new Image<double>(ImageSize(100, 100));
	ImagePtr	result(psf);
	double	n = 2 * stddev * stddev;
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 100; y++) {
			double	d = hypot(x - 50, y - 50);
			psf->pixel(x, y) = exp(-d * d / n);
		}
	}
	return result;
}

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
	std::string	prefix;
	bool	gauss = false;
	double	stddev = 0;
	bool	constrained = false;
	double	epsilon = 0;
	double	K = 0;
	while (EOF != (c = getopt_long(argc, argv, "cdgh?i:mp:P:s:k:",
		longopts, &longindex)))
		switch (c) {
		case 'c':
			constrained = true;
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			epsilon = std::stod(optarg);
			break;
		case 'g':
			gauss = true;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			iterations = std::stoi(optarg);
			break;
		case 'k':
			K = std::stod(optarg);
			break;
		case 'm':
			method = std::string(optarg);
			break;
		case 'p':
			{
			io::FITSin	psffile(optarg);
			psf = psffile.read();
			}
			break;
		case 'P':
			prefix = std::string(optarg);
			break;
		case 's':
			stddev = std::stod(optarg);
			break;
		}

	// if we have a stddev we can generate a psf
	if (gauss) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using gaussian psf");
		if (psf) {
			Image<double>	*img = dynamic_cast<Image<double>*>(&*psf);
			if (NULL == img) {
				std::string msg = stringprintf("can only process double psf");
				throw std::runtime_error(msg);
			}
			stddev = findstddev(*img);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "using stddev = %f", stddev);
		}
		psf = gausspsf(stddev);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using gaussian psf");
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
		FourierDeconvolutionOperator	fdco(psf);
		outimage = fdco(image);
	}
	if (method == std::string("pseudo")) {
		PseudoDeconvolutionOperator	pdco(psf);
		pdco.epsilon(epsilon);
		outimage = pdco(image);
	}
	if (method == std::string("wiener")) {
		WienerDeconvolutionOperator	wdco(psf);
		wdco.K(K);
		outimage = wdco(image);
	}
	if (method == std::string("vancittert")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "performing vancittert deconvolution");
		VanCittertOperator	vc(psf);
		vc.iterations(iterations);
		vc.prefix(prefix);
		vc.constrained(constrained);
		outimage = vc(image);
	}
	if (method == std::string("fastvancittert")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "performing fastvancittert deconvolution");
		FastVanCittertOperator	fvc(psf);
		fvc.iterations(iterations);
		fvc.prefix(prefix);
		fvc.constrained(constrained);
		outimage = fvc(image);
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

