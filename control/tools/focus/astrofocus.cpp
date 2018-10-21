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
{ "debug",	no_argument,		NULL,		'd' },
{ "method",	required_argument,	NULL,		'm' },
{ "rectangle",	required_argument,	NULL,		'r' },
{ "solver",	required_argument,	NULL,		's' },
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
	std::cout << prg << "[ options ] help" << std::endl;
	std::cout << prg << "[ options ] evaluate [ position image ... ]";
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
	FocusInputImages	images(input);
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

	int	c;
	int	longindex;
	putenv((char *)"POSIXLY_CORRECT=1");    // cast to silence compiler
	while (EOF != (c = getopt_long(argc, argv, "c:dhm:r:s:w", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			center = image::ImagePoint(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
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
