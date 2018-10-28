/*
 * astrofocus.cpp -- Process images and focus positions 
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroFormat.h>
#include <AstroIO.h>
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <AstroImage.h>
#include <AstroFocus.h>
#include <ImageWriter.h>
#include <string>
#include <list>

using namespace astro::focusing;

namespace astro {
namespace app {
namespace focus {

static ImageWriter::format_t	format = ImageWriter::FITS;

static std::string	prefix;

/**
 * \brief Auxiliary function to construct a file name
 */
std::string	build_filename(unsigned long position) {
	std::string	extension("fits");
	switch (format) {
	case ImageWriter::FITS:
		extension = std::string("fits");
	case ImageWriter::JPEG:
		extension = std::string("jpg");
	case ImageWriter::PNG:
		extension = std::string("png");
	}
	return stringprintf("%s-%08lu.%s", prefix.c_str(), position,
		extension.c_str());
}

/**
 * \brief Decide what format to use based on the file name
 */
void	set_format_from_filename(const std::string& filename) {
	if (JPEG::isjpegfilename(filename)) {
		format = ImageWriter::JPEG;
	} else if (PNG::ispngfilename(filename)) {
		format = ImageWriter::PNG;
	} else {
		format = ImageWriter::FITS;
	}
}

/**
 * \brief Write an image with the right type
 */
void	save_image(ImagePtr image, const std::string& filename) {
	try {
		switch (format) {
		case ImageWriter::FITS:
			{
				io::FITSout	out(filename);
				out.setPrecious(false);
				out.write(image);
			}
			break;
		case ImageWriter::JPEG:
			{
				image::JPEG	jpeg;
				jpeg.writeJPEG(image, filename);
			}
			break;
		case ImageWriter::PNG:
			{
				image::PNG	png;
				png.writePNG(image, filename);
			}
			break;
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write %s: %s",
			filename.c_str(), x.what());
	}
}

/**
 * \brief Auxiliary function to write a processed image
 */
void	save_image(ImagePtr image, unsigned long position) {
	std::string	filename = build_filename(position);
	save_image(image, filename);
}

/**
 * \brief Table of options for the astrofocus program
 */
static struct option	longopts[] = {
{ "center",	required_argument,	NULL,		'c' },
{ "ccd",	required_argument,	NULL,		'C' },
{ "debug",	no_argument,		NULL,		'd' },
{ "exposure",	required_argument,	NULL,		'e' },
{ "focuser",	required_argument,	NULL,		'F' },
{ "format",	required_argument,	NULL,		'f' },
{ "help",	no_argument,		NULL,		'h' },
{ "method",	required_argument,	NULL,		'm' },
{ "prefix",	required_argument,	NULL,		'p' },
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
	std::cout << " -C,--ccd=<ccd>       use CCD named <ccd>" << std::endl;
	std::cout << " -e,--exposure=<t>    use exposure time <t>" << std::endl;
	std::cout << " -F,--focuser=<f>     use focuser name <f>" << std::endl;
	std::cout << " -f,--format=<fmt>    produce processed images in format <fmt>, where <fmt>" << std::endl;
	std::cout << "                      can be fits, jpg or png" << std::endl;
	std::cout << " -m,--method=<m>      use <m> evaulation method"
		<< std::endl;
	std::cout << " -p,--prefix=<p>      prefix for processed files"
		<< std::endl;
	std::cout << " -s,--solve=<s>       use <s> solution method"
		<< std::endl;
	std::cout << " -S,--steps=<s>       divide the interval into <s> steps"
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
 * \brief Method
 */
int	image_command(const std::string& filename, const std::string&method,
		const ImageRectangle& rectangle,
		const std::string& processedfile) {
	// read the image
	io::FITSin	in(filename);
	ImagePtr	image = in.read();

	// apply the evaluator
	FocusEvaluatorPtr	evaluator = FocusEvaluatorFactory::get(method,
						rectangle);
	double	val = (*evaluator)(image);

	// display the results
	std::cout << "value: " << val << std::endl;

	// write the processed image
	if (processedfile.size() > 0) {
		set_format_from_filename(processedfile);
		save_image(evaluator->evaluated_image(), processedfile);
	}
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
 * \brief Method to actually evaluate the image data
 */
int	evaluate_command(FocusInput& input, const std::string& prefix) {
	// construct a processor
	FocusProcessor	processor(input);
	processor.keep_images(prefix != std::string());

	// process all the images
	processor.process(input);

	// get the output
	FocusOutputPtr	output = processor.output();

	// retrieve the evaluatation results
	FocusItems	focusitems = output->items();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d items", focusitems.size());
	std::for_each(focusitems.begin(), focusitems.end(),
		[](const FocusItem& p) {
			std::cout << p.position() << " " << p.value();
			std::cout << std::endl;
		}
	);

	// export all the processed images if we have a prefix
	if (prefix != std::string()) {
		std::for_each(output->begin(), output->end(),
			[=](const std::pair<unsigned long, FocusElement>& i) {
				if (!i.second.processed_image) return;
				save_image(i.second.processed_image, i.first);
			}
		);
	}

	// Do we have a solver?
	if (input.solver().size() == 0) {
		return EXIT_SUCCESS;
	}

	// proceed to perform the solution
	return solve_command(focusitems, input.solver());
}

/**
 * \brief Perform the focus process
 */
int	focus_command(const FocusParameters& parameters,
		const std::string& ccdname, const std::string& focusername) {
	// get the devices
	camera::CcdPtr	ccd;
	camera::FocuserPtr	focuser;
	try {
		module::Devices	_devices(module::getModuleRepository());
                ccd = _devices.getCcd(ccdname);
		focuser = _devices.getFocuser(focusername);
	} catch (const std::exception& x) {
		std::cerr << "cannot get devices: " << x.what() << std::endl;
		return EXIT_FAILURE;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got devices");

	// Construct a local focus process
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing process");
	FocusProcess	process(parameters, ccd, focuser);

	// construct the callback and install it
	if (prefix.size() > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "installing callback");
		callback::CallbackPtr	callback(new ImageWriter(prefix, format));
		process.callback(callback);
	}

	// start the process
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start process");
	process.start();

	// wait for the process to terminate
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for process");
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
	debug_set_ident("astrofocus");
	debugthreads = 1;
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
	while (EOF != (c = getopt_long(argc, argv,
			"C:c:df:F:hm:p:r:s:w?", longopts,
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
		case 'f':
			if (std::string("fits") == std::string(optarg)) {
				format = ImageWriter::FITS;
			}
			if ((std::string("jpeg") == std::string(optarg)) ||
				(std::string("jpg") == std::string(optarg))) {
				format = ImageWriter::JPEG;
			}
			if (std::string("png") == std::string(optarg)) {
				format = ImageWriter::PNG;
			}
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
		case 'p':
			prefix = std::string(optarg);
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

	// handle the 'image' command
	if (command == std::string("image")) {
		// next argument must be the filename
		if (optind >= argc) {
			std::cerr << "image file name missing" << std::endl;
			return EXIT_FAILURE;
		}
		std::string	imagename(argv[optind++]);
		std::string	processedfile;
		if (optind < argc) {
			processedfile = std::string(argv[optind++]);
		}
		return image_command(imagename, method, rectangle, processedfile);
	}

	// handle the 'evaluate' command
	if (command == std::string("evaluate")) {
		FocusInput	fi;
		fi.method(method);
		fi.solver(solver);
		if (rectangle != image::ImageRectangle()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "use rectangle %s",
				rectangle.toString().c_str());
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
		return evaluate_command(fi, prefix);
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

		// prepare the parameters
		FocusParameters	fp(minposition, maxposition);
		fp.steps(steps);
		camera::Exposure	exposure;
		exposure.exposuretime(exposuretime);
		exposure.purpose(camera::Exposure::focus);
		exposure.frame(rectangle);
		fp.exposure(exposure);
		fp.method(method);
		fp.solver(solver);

		// get the the devices
		return focus_command(fp, ccdname, focusername);
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
