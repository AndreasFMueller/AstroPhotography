/*
 * astrocooler.cpp
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
namespace cooler {

/**
 * \brief Display a usage message
 *
 * \param progname	programm name (for the help message)
 */
static void	usage(const char *progname) {
	std::cout << "get information and control a cooler" << std::endl;
	std::cout << std::endl;
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;

	std::cout << "    " << p.basename();
	std::cout << " [ options ] help" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <cooler>" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <cooler> { on | off } " << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <cooler> set <temperature> "
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
 * \brief Implementation of the on/off command
 *
 * \param cooler	the cooler to control
 * \param on_not_off	whether or not to turn the cooler on
 */
static int	onoff_command(astro::camera::CoolerPtr cooler,
			bool on_not_off) {
	cooler->setOn(on_not_off);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the get command
 *
 * \param cooler	the cooler to get information from
 */
static int	get_command(astro::camera::CoolerPtr cooler) {
	std::cout << stringprintf("actual=%.1f°C ", 
		cooler->getActualTemperature().celsius());
	std::cout << stringprintf("set=%.1f°C ",
		cooler->getSetTemperature().celsius());
	std::cout << ((cooler->isOn()) ? "ON" : "OFF");
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the set command
 *
 * \param cooler	the cooler to set
 * \param settemp	the set temperature to set the cooler to
 */
static int	set_command(astro::camera::CoolerPtr cooler,
			const Temperature& settemp) {
	cooler->setTemperature(settemp);
	return EXIT_SUCCESS;
}

/**
 * \brief The main method for the astrocooler program
 *
 * \param argc		the number of arguments
 * \param argv		the argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astrocooler");
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
	astro::device::DeviceAccessor<astro::camera::CoolerPtr>	access(repository);
	astro::camera::CoolerPtr	cooler = access.get(name);

	// if there are no more arguments, then the get command is implied
	if (optind >= argc) {
		if (interval > 0) {
			while (repeats--) {
				get_command(cooler);
				Timer::sleep(interval);
			}
		}
		return get_command(cooler);
	}
	command = std::string(argv[optind++]);

	// on/off command
	if (command == "on") {
		return onoff_command(cooler, true);
	}
	if (command == "off") {
		return onoff_command(cooler, false);
	}

	if (optind >= argc) {
		std::cerr << "missing temperature" << std::endl;
		return EXIT_FAILURE;
	}
	Temperature	temp(std::stod(argv[optind++]), Temperature::CELSIUS);
	if (command == "set") {
		return set_command(cooler, temp);
	}

	std::cerr << "unknown command: " << command << std::endl;
	return EXIT_FAILURE;
}

} // namespace cooler
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::cooler::main>(argc, argv);
}
