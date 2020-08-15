/*
 * astroguideport.cpp
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
namespace guideport {

/**
 * \brief Usage method
 *
 * \param progname	name of the program (for the help message)
 */
static void	usage(const char *progname) {
	std::cout << "get information and control a guideport" << std::endl;
	std::cout << std::endl;
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;

	std::cout << "    " << p.basename();
	std::cout << " [ options ] help" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <guideport>" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] <guideport> activate RA+ RA- DEC+ DEC-"
		<< std::endl;

	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "   -d,--debug          increase debug level" << std::endl;
	std::cout << "   -h,--help,-?        show this help message and exit"
		<< std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ NULL,		0,			NULL,	 	 0  }
};

/**
 * \brief Implementation of the (default) active command
 *
 * \param guideport	the guideport to connect to
 */
static int	active_command(camera::GuidePortPtr guideport) {
	uint8_t	a = guideport->active();
	if (a & camera::GuidePort::RAPLUS) {
		std::cout << "RA+ ";
	}
	if (a & camera::GuidePort::RAMINUS) {
		std::cout << "RA- ";
	}
	if (a & camera::GuidePort::DECPLUS) {
		std::cout << "DEC+ ";
	}
	if (a & camera::GuidePort::DECMINUS) {
		std::cout << "DEC+ ";
	}
	if (a != 0) {
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Activate guideport pins
 *
 * \param guideport	the guideport to activate
 * \param activation	the activation object to activate
 */
static int	activate_command(astro::camera::GuidePortPtr guideport,
			const camera::GuidePortActivation& activation) {
	float	sleeptime = activation.max() + 2;
	//Timer::sleep(1);
	guideport->activate(activation);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleeping for %.3f seconds", sleeptime);
	Timer::sleep(sleeptime);
	return EXIT_SUCCESS;
}

/**
 * \brief Maine method for the astroguideport program
 *
 * \param argc		number of arguments
 * \param argv		argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("astroguideport");
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
	astro::device::DeviceAccessor<astro::camera::GuidePortPtr>	access(repository);
	astro::camera::GuidePortPtr	guideport = access.get(name);

	// if there are no more arguments, then the get command is implied
	if (optind >= argc) {
		return active_command(guideport);
	}
	command = std::string(argv[optind++]);

	// on/off command
	if (command != std::string("activate")) {
		std::string	msg = stringprintf("command %s unknown",
			command.c_str());
		throw std::runtime_error(msg);
	}

	if (optind + 4 >  argc) {
		std::cerr << "missing timing arguments" << std::endl;
		return EXIT_FAILURE;
	}

	float raplus = std::stod(argv[optind++]);
	float raminus = std::stod(argv[optind++]);
	float decplus = std::stod(argv[optind++]);
	float decminus = std::stod(argv[optind++]);
	camera::GuidePortActivation	activation(raplus, raminus,
						decplus, decminus);
	return activate_command(guideport, activation);
}

} // namespace guideport
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::guideport::main>(argc, argv);
}
