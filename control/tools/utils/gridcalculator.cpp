/*
 * gridcalculator.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <getopt.h>

namespace astro {
namespace app {
namespace grid {

/**
 * \brief Help function
 *
 * \param progname	name of the program
 */
static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] " << std::endl;
	std::cout << std::endl;
	std::cout << "compute grid parameters for a starchart grid";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug         enter debug mode" << std::endl;
	std::cout << " -h,-?,--help       show this help message and exit"
		<< std::endl;
	std::cout << " -c,--center=<ra dec>  compute grid for center <ra dec>"
		<< std::endl;
}

/**
 * \brief Table of options for the astroephemeris program
 */
static struct option	options[] = {
{ "debug",		no_argument,			NULL,		'd' },
{ "help",		no_argument,			NULL,		'h' },
{ "center",		required_argument,		NULL,		'c' },
{ "resolution",		required_argument,		NULL,		'r' },
{ "frame",		required_argument,		NULL,		'f' },
{ "pixels",		required_argument,		NULL,		'p' },
{ NULL,			0,				NULL,		 0  }
};

/**
 * \brief Main function for the astroephemeris program
 *
 * \param argc  number of arguments
 * \param argv  vector of arguments
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	astro::RaDec	center;
	astro::Size	frame(1920,1080);
	double	pixels_per_degree = 100;
	double	pixelstep = 100;
	while (EOF != (c = getopt_long(argc, argv, "dh?c:r:f:p:", options,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'c':
			center = astro::RaDec(std::string(optarg));
			break;
		case 'r':
			pixels_per_degree = std::stod(optarg);
			break;
		case 'f':
			frame = Size(std::string(optarg));
			break;
		case 'p':
			pixelstep = std::stod(optarg);
			break;
		}
	}

	std::cout << "Input:" << std::endl;
	std::cout << "center:         " << center.toString() << std::endl;
	std::cout << "pixels/degrees: " << pixels_per_degree << std::endl;
	std::cout << "frame size:     " << frame.toString() << std::endl;
	std::cout << "pixelstep:      " << pixelstep << std::endl;

	astro::utils::GridCalculator	gridcalculator(center, frame,
						pixels_per_degree);
	gridcalculator.gridsetup(pixelstep);

	std::cout << "Zero:      " << gridcalculator.gridzero().toString();
	std::cout << std::endl;
	std::cout << "Steps:     " << gridcalculator.stepsizes().toString();
	std::cout << std::endl;
	std::cout << "RA range:  " << gridcalculator.minra() << " -- ";
	std::cout << gridcalculator.maxra() << std::endl;
	std::cout << "DEC range: " << gridcalculator.mindec() << " -- ";
	std::cout << gridcalculator.maxdec() << std::endl;

	for (int ra = gridcalculator.minra(); ra <= gridcalculator.maxra();
		ra++) {
		for (int dec = gridcalculator.mindec();
			dec <= gridcalculator.maxdec(); dec++) {
			std::cout << "grid point ";
			std::cout << stringprintf("%3d,%3d: ", ra, dec);
			std::cout << gridcalculator.gridpoint(ra, dec)
					.toString();
			std::cout << std::endl;
		}
	}
	std::cout << "drawing grids:" << std::endl;
	for (int ra = gridcalculator.minra(); ra <= gridcalculator.maxra();
		ra++) {
		std::cout << "DEC range ";
		std::cout << stringprintf("ra_i=%3d", ra);
		std::cout << ": ";
		std::cout << gridcalculator.angleRangeDEC(ra).toString(
				Angle::Degrees);
		std::cout << std::endl;
	}
	for (int dec = gridcalculator.mindec(); dec <= gridcalculator.maxdec();
		dec++) {
		std::cout << "RA range ";
		std::cout << stringprintf("dec_i=%3d", dec);
		std::cout << ": ";
		std::cout << gridcalculator.angleRangeRA(dec).toString(
				Angle::Hours);
		std::cout << std::endl;
	}
	
	return EXIT_SUCCESS;
}

} // namespace grid
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::app::grid::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by exception: " << x.what();
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
}

