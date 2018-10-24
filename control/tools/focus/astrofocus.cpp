/*
 * astrofocus.cpp -- Process images and focus positions 
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroFormat.h>
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <AstroImage.h>
#include <AstroFocus.h>
#include <string>
#include <list>

using namespace astro::focusing;

namespace astro {
namespace app {
namespace focus {

/**
 * \brief Table of options for the astrofocus program
 */
static struct option	longopts[] = {
{ "center",	required_argument,	NULL,		'c' },
{ "ccd",	required_argument,	NULL,		'C' },
{ "debug",	no_argument,		NULL,		'd' },
{ "exopsure",	required_argument,	NULL,		'e' },
{ "focuser",	required_argument,	NULL,		'F' },
{ "help",	no_argument,		NULL,		'h' },
{ "method",	required_argument,	NULL,		'm' },
{ "rectangle",	required_argument,	NULL,		'r' },
{ "solver",	required_argument,	NULL,		's' },
{ "steps",	required_argument,	NULL,		'S' },
{ "window",	required_argument,	NULL,		'w' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief Help function
 */
static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] help" << std::endl;
	std::cout << prg << " [ options ] evaluate [ position image ... ]";
	std::cout << std::endl;
	std::cout << prg << " [ options ] solve [ position value ...]";
	std::cout << std::endl;
	std::cout << prg << " [ options ] focus min max";
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -h,--help            display this help message and exit"
		<< std::endl;
	std::cout << " -c,--center=<c>      restrict to a window around the point <c>"
		<< std::endl;
	std::cout << " -m,--method=<m>      use <m> evaulation method"
		<< std::endl;
	std::cout << " -s,--solve=<s>       use <s> solution method"
		<< std::endl;
	std::cout << " -r,--rectangle<r>    only take contents of rectangle <r>"
		" into acount."
		<< std::endl;
	std::cout << "                      The rectangle must be specified as"
		<< std::endl;
	std::cout << "                      widthxheight@(xoffset,yoffset)."
		<< std::endl;
	std::cout << " -w,--window=<w>      window dimensions widthxheight"
		<< std::endl;
}

/**
 * \brief Method to actually evaluate the image data
 */
int	evaluate_command(const FocusInput& input) {
	// construct a processor
	FocusProcessor	processor(input);
	processor.keep_images(false);

	// process all the images
	processor.process(input);

	// retrieve the evaluatation results
	FocusItems	focusitems = processor.output()->items();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d items", focusitems.size());
	std::for_each(focusitems.begin(), focusitems.end(),
		[](const FocusItem& p) {
			std::cout << p.position() << " " << p.value();
			std::cout << std::endl;
		}
	);

	return EXIT_SUCCESS;
}

/**
 * \brief Method to compute solution from a solver
 *
 * \param items		focus items to use as absis for the solution
 */
int	solve_command(const FocusItems& items, const std::string& solver) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "solving %d items, with %s",
		items.size(), solver.c_str());
	FocusSolverPtr	solverptr = FocusSolverFactory::get(solver);
	int	solution = solverptr->position(items);
	std::cout << "position: " << solution << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Perform the focus process
 */
int	focus_command(unsigned long minposition, unsigned long maxposition,
		int steps, double exposuretime,
		const std::string& ccdname, const std::string& focusername,
		const std::string& method, const std::string& solver) {
	// get the devices
	camera::CcdPtr	ccd;
	camera::FocuserPtr	focuser;

	// Construct a local focus process
	BasicFocusProcess	process(ccd, focuser);

	// set all the parameters
	process.minposition(minposition);
	process.maxposition(maxposition);
	camera::Exposure	exposure;
	exposure.exposuretime(exposuretime);
	process.exposure(exposure);
	process.steps(steps);
	process.method(method);
	process.solver(solver);

	// XXX install a callback for reporting

	// start the process
	process.start();

	// wait for the process to terminate
	process.wait();

	// XXX report the results of the process

	// that's it
	return EXIT_SUCCESS;
}

/**
 * \brief Main function for the astrofocus program
 *
 * \param argc	number of arguments
 * \param argv	vector of arguments
 */
int	main(int argc, char *argv[]) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus utility");
	image::ImagePoint	center;
	image::ImageRectangle	rectangle;
	image::ImageSize	window;
	std::string	method("fwhm");
	std::string	solver("abs");
	std::string	ccdname;
	std::string	focusername;
	double	exposuretime = 1;
	int	steps = 10;
	int	c;
	int	longindex;
	putenv((char *)"POSIXLY_CORRECT=1");    // cast to silence compiler
	while (EOF != (c = getopt_long(argc, argv, "c:dhm:r:s:w?", longopts,
		&longindex))) {
		switch (c) {
		case 'C':
			ccdname = std::string(optarg);
			break;
		case 'c':
			center = image::ImagePoint(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposuretime = std::stod(optarg);
			break;
		case 'F':
			focusername = std::string(optarg);
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'm':
			method = std::string(optarg);
			break;
		case 'r':
			rectangle = image::ImageRectangle(optarg);
			break;
		case 's':
			solver = std::string(optarg);
			break;
		case 'S':
			steps = std::stoi(optarg);
			break;
		case 'w':
			window = image::ImageSize(optarg);
			break;
		}
	}

	// get the rectangle from center and window, if present
	if (center != image::ImagePoint()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing rectangle");
		if (window == image::ImageSize()) {
			window = image::ImageSize(256, 256);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "window: %s",
			window.toString().c_str());
		image::ImagePoint	origin(center.x() - window.width() / 2,
					center.y() - window.height() / 2);
		rectangle = image::ImageRectangle(origin, window);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Rectangle: %s",
		rectangle.toString().c_str());

	// get the command
	if (argc < optind + 1) {
		throw std::runtime_error("no command specified, try help");
	}
	std::string	command(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing command '%s'",
		command.c_str());

	// handle the 'help' command
	if (command == std::string("help")) {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	// handle the 'evaluate' command
	if (command == std::string("evaluate")) {
		FocusInput	fi;
		fi.method(method);
		fi.solver(solver);
		if (rectangle != image::ImageRectangle()) {
			fi.rectangle(rectangle);
		}
		// convert into a list of position and file names
		while (optind < argc - 1) {
			std::string	pos(argv[optind++]);
			unsigned long	position = std::stoi(pos);
			std::string	filename(argv[optind++]);
			fi.insert(std::make_pair(position, filename));
		}
		if (optind < argc) {
			std::cerr << "incorrect number of arguments";
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		std::cout << "Focus processing for files:" << std::endl;
		std::cout << fi.toString();
		return evaluate_command(fi);
	}


	// handle the 'solve' command
	if (command == std::string("solve")) {
		FocusItems	items;
		while (optind < argc - 1) {
			int	pos = std::stoi(argv[optind++]);
			double	val = std::stod(argv[optind++]);
			items.insert(FocusItem(pos, val));
		}
		if (optind < argc) {
			std::cerr << "incorrect number of arguments";
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		return solve_command(items, solver);
	}

	// handle the 'focus' command
	if (command == std::string("focus")) {
		if (argc >= optind) {
			std::runtime_error("not enough arguments");
		}
		unsigned long	minposition = std::stoi(argv[optind++]);
		if (argc >= optind) {
			std::runtime_error("not enough arguments");
		}
		unsigned long	maxposition = std::stoi(argv[optind++]);

		// get the the devices
		return focus_command(minposition, maxposition, steps,
			exposuretime, ccdname, focusername,
			method, solver);
	}

	// handle unknown commands
	std::cerr << "unknown command '" << command << "'" << std::endl;
	return EXIT_FAILURE;
}

} // namespace focus
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::focus::main>(argc, argv);
}
