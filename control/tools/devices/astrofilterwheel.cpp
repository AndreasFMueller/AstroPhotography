/*
 * astrofilterwheel.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroDevice.h>
#include <AstroLocator.h>
#include <AstroDevaccess.h>

namespace astro {
namespace app {
namespace filterwheel {

/**
 * \brief Display a usage message
 *
 * \param progname	the name of the program (for the command line)
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <filterwheel> list " << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <filterwheel> goto <n>" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <filterwheel> exercise" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "   -d,--debug       increase debug level" << std::endl;
	std::cout << "   -h,--help,-?     show this help message and exit";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,	NULL,	'd' },
{ "help",	no_argument,	NULL,	'h' },
{ NULL,		0,		NULL,	 0  }
};

/**
 * \brief Implementation of the list command
 *
 * The list command lists the available filter wheels and their positions
 *
 * \param filterwheel	the filterwheel to handle
 */
static int	list_command(astro::camera::FilterWheelPtr filterwheel) {
	size_t	nfilters = filterwheel->nFilters();
	for (size_t position = 0; position < nfilters; position++) {
		std::cout << position << ": ";
		std::cout << filterwheel->filterName(position);
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the goto command
 *
 * \param filterwheel	the filterwheel to move
 * \param position	the position to move to
 */
static int	goto_command(astro::camera::FilterWheelPtr filterwheel,
			const std::string& position) {
	try {
		size_t	pos = std::stod(position);
		if (pos >= filterwheel->nFilters()) {
			std::cerr << "invalid position number: " << pos;
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		filterwheel->select(pos);
		filterwheel->wait(60);
		return EXIT_SUCCESS;
	} catch (std::exception) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not a numeric position");
	}
	try {
		filterwheel->select(position);
		filterwheel->wait(60);
	} catch (std::exception) {
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the exercise command
 *
 * The exercise command moves the filterwheel around through all positions
 * and terminates.
 *
 * \param filterwheel	the filterwheel to exercise
 */
static int	exercise_command(astro::camera::FilterWheelPtr filterwheel) {
	filterwheel->wait(60);
	std::cout << "number of filters: " << filterwheel->nFilters();
	std::cout << std::endl;
	for (int i = filterwheel->nFilters() - 1; i >= 0; i--) {
		filterwheel->select(i);
		filterwheel->wait(60);
		std::cout << "filter position: " << filterwheel->currentPosition();
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief The main function for the astrofilterwheel program
 *
 * \param argc		the number of arguments
 * \param argv		the argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astrofilterwheel");
	debugthreads = 1;
	debugtimeprecision = 3;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

	if (optind >= argc) {
		std::cerr << "not enough arguments" << std::endl;
		return EXIT_FAILURE;
	}
	DeviceName	name(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working with device name: %s",
		name.toString().c_str());

	// create a repository
	auto	repository = module::ModuleRepository::get();
	astro::device::DeviceAccessor<astro::camera::FilterWheelPtr>	access(repository);
	astro::camera::FilterWheelPtr	filterwheel = access.get(name);

	// wait for the filterwheel to be idle
	filterwheel->wait(60);

	// next argument must be a command
	if (optind >= argc) {
		std::cerr << "no command" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	command(argv[optind++]);

	if (command == "list") {
		return list_command(filterwheel);
	}

	if (command == "exercise") {
		return exercise_command(filterwheel);
	}

	if (command != "goto") {
		std::cerr << "unknown command: " << command << std::endl;
		return EXIT_FAILURE;
	}
	if (optind >= argc) {
		std::cerr << "missing filter position or name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	filtername(argv[optind++]);

	return goto_command(filterwheel, filtername);
}

} // namespace filterwheel
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::filterwheel::main>(argc, argv);
}
