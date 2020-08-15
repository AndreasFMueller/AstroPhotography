/*
 * snowccd.cpp -- query or operate a ccd
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <camera.h>
#include <IceConversions.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <AstroConfig.h>
#include <includes.h>
#include <AstroFormat.h>
#include <AstroIO.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowccd {

/**
 * \brief display a help message
 *
 * \param progname	the program name used in the help message
 */
static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> <ccd>" << std::endl;
	std::cout << p << " [ options ] <server> <ccd> <time> <file>"
		<< std::endl;
	std::cout << std::endl;
}


static struct option	longopts[] = {
{ "debug",	no_argument,			NULL,		'd' },
{ "help",	no_argument,			NULL,		'h' },
{ NULL,		0,				NULL,		 0  }
};

/**
 * \brief List all ccd devices known to the server
 *
 * \param devices	the devices module to query for ccds
 */
int	command_list(DevicesPrx devices) {
	DeviceNameList  list = devices->getDevicelist(DevCCD);
	std::for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Display information about a ccd
 *
 * \param ccd	the ccd to query
 */
int	command_info(CcdPrx ccd) {
	std::cout << "name:        " << ccd->getName() << std::endl;
	std::cout << "info:        " << convert(ccd->getInfo()).toString();
	std::cout << std::endl;
	std::cout << "has gain:    " << ((ccd->hasGain()) ? "yes" : "no");
	std::cout << std::endl;
	if (ccd->hasGain()) {
		std::cout << "gain:        " << ccd->getGain() << std::endl;
	}
	std::cout << "has shutter: " << ((ccd->hasShutter()) ? "yes" : "no");
	std::cout << std::endl;
	if (ccd->hasShutter()) {
		std::cout << "shutter:     ";
		std::cout << astro::camera::Shutter::state2string(
				convert(ccd->getShutterState()));
		std::cout << std::endl;
	}
	std::cout << "has cooler:  ";
	std::cout << ((ccd->hasCooler()) ? "yes" : "no");
	std::cout << std::endl;
	if (ccd->hasCooler()) {
		auto cooler = ccd->getCooler();
		std::cout << "cooler:      " << cooler->getName();
		std::cout << std::endl;
		std::cout << "cooler state:";
		std::cout << ((cooler->isOn()) ? "on" : "off");
		std::cout << std::endl;
		astro::Temperature	set(cooler->getActualTemperature());
		std::cout << astro::stringprintf("act temp:    %.1f°C",
			set.celsius());
		std::cout << std::endl;
		astro::Temperature	act(cooler->getSetTemperature());
		std::cout << astro::stringprintf("set temp:    %.1f°C",
			act.celsius());
		std::cout << std::endl;
	}
	std::cout << "state:       ";
	std::cout << astro::camera::CcdState::state2string(
			convert(ccd->exposureStatus()));
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

int	command_image(CcdPrx ccd, const astro::camera::Exposure& exposure,
		const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure %s",
		exposure.toString().c_str());
	ccd->startExposure(convert(exposure));
	ExposureState	state = ccd->exposureStatus();
	while (state == EXPOSING) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state: %s",
			astro::camera::CcdState::state2string(convert(state)).c_str());
		astro::Timer::sleep(1);
		state = ccd->exposureStatus();
	}
	switch (state) {
	case IDLE:
	case EXPOSING:
	case CANCELLING:
	case STREAMING:
	case BROKEN:
		return EXIT_FAILURE;
	case EXPOSED:
		break;
	}
	astro::io::FITSout	out(filename);
	out.setPrecious(false);
	out.write(convert(ccd->getImage()));
	return EXIT_SUCCESS;
}

class CcdCallbackI : public CcdCallback {
public:
	virtual void	state(ExposureState s, const Ice::Current& current) {
		std::cout << astro::camera::CcdState::state2string(convert(s));
		std::cout << std::endl;
	}
	virtual void	stop(const Ice::Current& /* current */) {
		exit(EXIT_SUCCESS);
	}
};

int	command_monitor(CcdPrx ccd) {
	CommunicatorSingleton::connect(ccd);
	CcdCallbackI	*_callback = new CcdCallbackI();
	Ice::ObjectPtr	callbackptr = _callback;
	Ice::Identity	identity = CommunicatorSingleton::add(callbackptr);
	ccd->registerCallback(identity);
	sleep(86400);
	ccd->unregisterCallback(identity);
	return EXIT_SUCCESS;
}

/**
 * \brief The main function for the snowccd program
 *
 * \param argc		the number of arguments
 * \param argv		the argumetn vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowccd");
	CommunicatorSingleton	comminicator(argc, argv);

	int	c;
	int	longindex;
	astro::ServerName	servername;
	astro::camera::Exposure	exposure;
	putenv("POSIXLY_CORRECT=1");
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts, &longindex)))
		switch(c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			break;
		}

	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	if (command == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Devices"));
	DevicesPrx	devices = DevicesPrx::checkedCast(base);

	if (command == "list") {
		return command_list(devices);
	}

	std::string	ccdname = command;
	CcdPrx	ccd = devices->getCcd(ccdname);
	if (optind >= argc) {
		return command_info(ccd);
	}

	if (optind < argc) {
		command = std::string(argv[optind++]);
	}
	if (command == "monitor") {
		return command_monitor(ccd);
	}

	if (optind >= argc) {
		throw std::runtime_error("not enought arguments");
	}

	exposure.exposuretime(std::stod(command));
	std::string	filename(argv[optind++]);
	return command_image(ccd, exposure, filename);

}

} // namepsace snowccd
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowccd::main>(argc, argv);
	CommunicatorSingleton::release();
	return rc;
}
