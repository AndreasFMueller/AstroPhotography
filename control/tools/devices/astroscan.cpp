/*
 * astroscan.cpp -- scan the module directory and list the available devices
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <getopt.h>
#include <includes.h>

namespace astro {
namespace app {
namespace scan {

/**
 * \brief list of options for the astroscan command
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ "verbose",	no_argument,		NULL,		'v' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief usage message for the astroscan program
 *
 * \param progname	the name of the program (for the help message)
 */
static void	usage(const char *progname) {
	Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] modules" << std::endl;
	std::cout << p << " [ options ] scan <module>" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << std::endl;
	std::cout << " -d,--debug    increase the debug level" << std::endl;
	std::cout << " -h,--help     display this help message and exit"
		<< std::endl;
	std::cout << " -v,--verbose  verbose display" << std::endl;
}

/**
 * \brief Modules command
 *
 * \param verbose	whether or not to use verbose display
 */
static int	modules_command(bool verbose) {
	auto _repository = module::ModuleRepository::get();
	// show information about the repository
	if (verbose) {
		std::cout << "repository path: ";
		std::cout << _repository->path();
		std::cout << std::endl;
	}

	// construct a list of module names
	std::vector<std::string>	modules = _repository->moduleNames();
	for_each(modules.begin(), modules.end(),
		[_repository,verbose](const std::string& modulename) {
			if (!verbose) {
				std::cout << modulename << std::endl; 
				return;
			}
			auto	module = _repository->getModule(modulename);
			std::cout << module->ctime().toString();
			std::cout << stringprintf("  %10lu  ", module->size());
			std::cout << stringprintf("%-20.20s",
				module->basename().c_str());
			std::cout << modulename;
			std::cout << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Auxiliary function to display a list of names
 */
static void	showlist(const std::vector<std::string>& names) {
	for_each(names.begin(), names.end(),
		[](const std::string& name) {
			std::cout << " " << name;
		}
	);
}

/**
 * \brief implementation of the scan command
 *
 * \param modulename	the module name
 * \param verbose	whether the display should be verbose
 */
static int	scan_command(const std::string& modulename, bool verbose) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scanning module '%s'",
		modulename.c_str());
	auto	_repository = module::ModuleRepository::get();

	// retrieve the module
	module::ModulePtr	module = _repository->getModule(modulename);
	if (NULL == module) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get module '%s'",
			modulename.c_str());
		return EXIT_FAILURE;
	}

	// get the module descriptor
	module::ModuleDescriptor	*moddesc = module->getDescriptor();
	if (NULL == moddesc) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get a module "
			"descriptor for '%s'", modulename.c_str());
		return EXIT_FAILURE;
	}

	// show the module information
	if (verbose) {
		std::cout << "module " << modulename << " version ";
		std::cout << moddesc->version() << std::endl;
	}

	// Check whether we have a device locator (some modules don't)
	if (!moddesc->hasDeviceLocator()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get a device "
			"locator for '%s'", modulename.c_str());
		return EXIT_FAILURE;
	}
	device::DeviceLocatorPtr	devloc = module->getDeviceLocator();

	// now tet the various types of objects
	std::cout << "adaptive optics:  ";
	showlist(devloc->getDevicelist(DeviceName::AdaptiveOptics));
	std::cout << std::endl;

	std::cout << "camera:           ";
	showlist(devloc->getDevicelist(DeviceName::Camera));
	std::cout << std::endl;

	std::cout << "ccd:              ";
	showlist(devloc->getDevicelist(DeviceName::Ccd));
	std::cout << std::endl;

	std::cout << "cooler:           ";
	showlist(devloc->getDevicelist(DeviceName::Cooler));
	std::cout << std::endl;

	std::cout << "guideport:        ";
	showlist(devloc->getDevicelist(DeviceName::Guideport));
	std::cout << std::endl;

	std::cout << "filterwheel:      ";
	showlist(devloc->getDevicelist(DeviceName::Filterwheel));
	std::cout << std::endl;

	std::cout << "focuser:          ";
	showlist(devloc->getDevicelist(DeviceName::Focuser));
	std::cout << std::endl;

	std::cout << "mount:            ";
	showlist(devloc->getDevicelist(DeviceName::Mount));
	std::cout << std::endl;

	return EXIT_SUCCESS;
}

/**
 * \brief Main function for the astroscan program
 *
 * \param argc	the number of arguments
 * \param argv	the verctor of arguments
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astroscan");
	bool	verbose = false;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dhv",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'v':
			verbose = true;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// make sure there is an additional argument
	if (optind >= argc) {
		std::cerr << "no command argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	command(argv[optind++]);

	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}
	if (command == "modules") {
		return modules_command(verbose);
	}
	std::list<std::string>	modulelist;
	while (optind < argc) {
		modulelist.push_back(std::string(argv[optind++]));
	}
	if (command == "scan") {	
		for_each(modulelist.begin(), modulelist.end(),
			[verbose](const std::string& modulename) {
				scan_command(modulename, verbose);
			}
		);
		return EXIT_SUCCESS;
	}
	std::cerr << "unknown command " << command << std::endl;
	return EXIT_FAILURE;
}


} // namespace scan
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::scan::main>(argc, argv);
}
