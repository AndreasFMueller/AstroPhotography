/*
 * guiderport.cpp -- utility program to test the guider port
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <getopt.h>
#include <AstroLoader.h>
#include <includes.h>

using namespace astro;

namespace guiderport {

void	usage(const char *progname) {
	std::string	p = "    " + Path(progname).basename();
	std::cout << p << " <guiderportname> <prognumber>" << std::endl;
	std::cout << std::endl;
	std::cout << "open the guider port and activate the outputs in one of the programs" << std::endl;
	std::cout << "identified by the program number. The following programs are available:" << std::endl;
	std::cout << "   1: activate each output for 1 second" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug      increase debug level" << std::endl;
	std::cout << "  -h,--help       display this help message and exit";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,	NULL,	'd' },
{ "help",	no_argument,	NULL,	'h' },
{ NULL,		0,		NULL,	0   }
};

void	prog0(astro::camera::GuiderPortPtr guiderport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting program 1");
	while (1) {
		guiderport->activate(0.5, 0, 0, 0);
		sleep(2);
		guiderport->activate(0, 0.5, 0, 0);
		sleep(2);
		guiderport->activate(0, 0, 0.5, 0);
		sleep(2);
		guiderport->activate(0, 0, 0, 0.5);
		sleep(3);
	}
}

int	main(int argc, char *argv[]) {

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			debugthreads = 1;
			debugtimeprecision = 3;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

	// next argument must be the device name
	if (argc <= optind) {
		throw std::runtime_error("no device specified");
	}
	DeviceName	devicename(argv[optind++]);

	// retrieve the guider port
	astro::module::Repository	repository;
        astro::module::Devices	devices(repository);

	astro::camera::GuiderPortPtr	guiderport
		= devices.getGuiderPort(devicename);

	// get the program number
	int	prognumber = 0;
	if (argc > optind) {
		prognumber = std::stoi(argv[optind++]) - 1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run program %d on %s", prognumber,
		devicename.toString().c_str());

	// run the different programs
	switch (prognumber) {
	case 0:
		prog0(guiderport);
		break;
	default:
		throw std::runtime_error("unknown program number");
	}

	return EXIT_SUCCESS;
}

} // guiderport

int	main(int argc, char *argv[]) {
	return astro::main_function<guiderport::main>(argc, argv);
}
