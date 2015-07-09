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

namespace astro {
namespace app {
namespace guiderport {

void	usage(const char *progname) {
	std::string	p = "    " + Path(progname).basename();
	std::cout << p << " <guiderportname> <prognumber>" << std::endl;
	std::cout << std::endl;
	std::cout << "open the guider port and activate the outputs in one of the programs" << std::endl;
	std::cout << "identified by the program number. The following programs are available:" << std::endl;
	std::cout << "   1: activate each output for 1 second in the order" << std::endl;
	std::cout << "      RA+, RA-, DEC+, DEC-" << std::endl;
	std::cout << "   2: do binary count using the port bits in increased significance" << std::endl;
	std::cout << "      as RA+, RA-, DEC+, DEC-" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "   3: " << std::endl;
	std::cout << "   4: RA backlash calibration 3s RA+, 3s RA-" << std::endl;
	std::cout << "   5: DEC backlash calibration 3s DEC+, 3s DEC-" << std::endl;
	std::cout << "  -d,--debug      increase debug level" << std::endl;
	std::cout << "  -h,--help       display this help message and exit";
	std::cout << "  -s,--scale=s    scale all times by the factor s";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,	'd' },
{ "help",	no_argument,		NULL,	'h' },
{ "scale",	required_argument,	NULL,	's' },
{ NULL,		0,			NULL,	 0   }
};

static float scale = 1;

void	prog0(astro::camera::GuiderPortPtr guiderport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting program 1");
	int	pause = scale + 2;
	while (1) {
		guiderport->activate(scale, 0, 0, 0);
		sleep(pause);
		guiderport->activate(0, scale, 0, 0);
		sleep(pause);
		guiderport->activate(0, 0, scale, 0);
		sleep(pause);
		guiderport->activate(0, 0, 0, scale);
		sleep(pause);
	}
}

void	prog1(astro::camera::GuiderPortPtr guiderport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting program 2");
	int	pause = scale;
	while (1) {
		for (int i = 0; i < 16; i++) {
			float	raplus = (i & 1) ? scale : 0;
			float	raminus = (i & 2) ? scale : 0;
			float	decplus = (i & 4) ? scale : 0;
			float	decminus = (i & 8) ? scale : 0;
			guiderport->activate(raplus, raminus,
				decplus, decminus);
			sleep(pause);
		}
		sleep(1);
	}
}

void	prog2(astro::camera::GuiderPortPtr guiderport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting program 3");
	int	pause = 5 * scale;
	while (1) {
		guiderport->activate(4 * scale, 3 * scale, 2 * scale, 1 * scale);
		sleep(pause);
		guiderport->activate(3 * scale, 2 * scale, 1 * scale, 4 * scale);
		sleep(pause);
		guiderport->activate(2 * scale, 1 * scale, 4 * scale, 3 * scale);
		sleep(pause);
		guiderport->activate(1 * scale, 4 * scale, 3 * scale, 2 * scale);
		sleep(pause);
	}
}

void	prog3(astro::camera::GuiderPortPtr guiderport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting RA backlash calibration");
	int	pause = scale;
	if (pause == 0) {
		pause = 1;
	}
	while (1) {
		guiderport->activate(pause,0,0,0);
		sleep(pause);
		guiderport->activate(0,pause,0,0);
		sleep(pause);
	}
}

void	prog4(astro::camera::GuiderPortPtr guiderport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting DEC backlash calibration");
	int	pause = scale;
	if (pause == 0) {
		pause = 1;
	}
	while (1) {
		guiderport->activate(0,0,pause,0);
		sleep(pause);
		guiderport->activate(0,0,0,pause);
		sleep(pause);
	}
}

typedef void (*program_t)(astro::camera::GuiderPortPtr);
program_t	programtable[5] = {
	prog0, prog1, prog2, prog3, prog4
};

int	main(int argc, char *argv[]) {

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dhs:", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			debugthreads = 1;
			debugtimeprecision = 3;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 's':
			scale = atof(optarg);
			if (scale <= 0) {
				throw std::runtime_error("scale must be positive");
			}
			break;
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
	if ((prognumber > 4) || (prognumber < 0)) {
		throw std::runtime_error("unknown program number");
	}
	programtable[prognumber](guiderport);

	return EXIT_SUCCESS;
}

} // namespace guiderport
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::guiderport::main>(argc, argv);
}
