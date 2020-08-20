/*
 * astrofocuser.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroDevice.h>
#include <AstroLocator.h>
#include <AstroDevaccess.h>

namespace astro {
namespace app {
namespace focuser {

/**
 * \brief Display a usage message
 *
 * \param progname	programm name (for the help message)
 */
static void	usage(const char *progname) {
	std::cout << "get information and control a focuser" << std::endl;
	std::cout << std::endl;
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;

	std::cout << "    " << p.basename();
	std::cout << " [ options ] help" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <focuser> [ get ]" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <focuser> info" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <focuser> set <location> "
		<< std::endl;

	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "   -d,--debug          increase debug level" << std::endl;
	std::cout << "   -h,--help,-?        show this help message and exit"
		<< std::endl;
	std::cout << "   -h,--interval=<i>   repeat status information at "
		"this interval" << std::endl;
	std::cout << "   -r,--repeats=<n>    repeat command <n> times"
		<< std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ "interval",	required_argument,	NULL,		'i' },
{ "repeats",	required_argument,	NULL,		'r' },
{ NULL,		0,			NULL,	 	 0  }
};

/**
 * \brief Implementation of the get command
 *
 * \param focuser	the focuser to get information from
 */
static int	get_command(astro::camera::FocuserPtr focuser, int interval,
			int repeats) {
	if (interval > 0) {
		while (repeats--) {
			std::cout << focuser->current();
			std::cout << std::endl;
			Timer::sleep(interval);
		}
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the set command
 *
 * \param focuser	the focuser to set
 * \param settemp	the set temperature to set the focuser to
 */
static int	set_command(astro::camera::FocuserPtr focuser,
			const long position) {
	focuser->set(position);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the info command
 *
 * \param focuser	the focuser to get the info from
 */
static int	info_command(astro::camera::FocuserPtr focuser) {
	std::cout << "name:        " << focuser->userFriendlyName()
		<< std::endl;
	std::cout << "backlash:    " << focuser->backlash() << std::endl;
	std::cout << "minimum:     " << focuser->min() << std::endl;
	std::cout << "maximum:     " << focuser->max() << std::endl;
	std::cout << "current:     " << focuser->current() << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief The main method for the astrofocuser program
 *
 * \param argc		the number of arguments
 * \param argv		the argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astrofocuser");
	debugthreads = 1;
	debugtimeprecision = 3;
	int	c;
	int	longindex;
	int	interval = 1;
	int	repeats = std::numeric_limits<int>::max();
	while (EOF != (c = getopt_long(argc, argv, "dh?i:r:", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			interval = std::stoi(optarg);
			break;
		case 'r':
			repeats = std::stoi(optarg);
			break;
		}

	if (optind >= argc) {
		std::cerr << "not enough arguments" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	command(argv[optind++]);

	// handle the help case
	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	DeviceName	name(command);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working with device name: %s",
		name.toString().c_str());


	// create a repository
	auto	repository = module::ModuleRepository::get();
	astro::device::DeviceAccessor<astro::camera::FocuserPtr>	access(repository);
	astro::camera::FocuserPtr	focuser = access.get(name);

	// if there are no more arguments, then the get command is implied
	if (optind >= argc) {
		return get_command(focuser, interval, repeats);
	}
	command = std::string(argv[optind++]);
	if (command == "get") {
		return get_command(focuser, interval, repeats);
	}
	if (command == "info") {
		return info_command(focuser);
	}

	if (optind >= argc) {
		std::cerr << "missing position" << std::endl;
		return EXIT_FAILURE;
	}
	long	position = std::stoi(argv[optind++]);
	if (command == "set") {
		return set_command(focuser, position);
	}

	std::cerr << "unknown command: " << command << std::endl;
	return EXIT_FAILURE;
}

} // namespace focuser
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::focuser::main>(argc, argv);
}
