/*
 * snowcooler.cpp -- query or operate a cooler
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

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowcooler {

/**
 * \brief Display a help message
 *
 * \param progname	the name of the program
 */
static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> <cooler> monitor" << std::endl;
	std::cout << p << " [ options ] <server> <cooler> info" << std::endl;
	std::cout << p << " [ options ] <server> <cooler> { on | off }"
		<< std::endl;
	std::cout << p << " [ options ] <server> <cooler> set <temperature>"
		<< std::endl;
	std::cout << p << " [ options ] <server> <cooler> dewheater <value>"
		<< std::endl;
	std::cout << std::endl;
}


static struct option	longopts[] = {
{ "debug",	no_argument,			NULL,		'd' },
{ "help",	no_argument,			NULL,		'h' },
{ NULL,		0,				NULL,		 0  }
};

/**
 * \brief Implementation of the help command
 *
 * \param progname	name of the program
 */
int	command_help(const std::string& progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation the list command
 *
 * \param devices	the devices proxy to query for coolers
 */
int	command_list(DevicesPrx devices) {
	DeviceNameList  list = devices->getDevicelist(DevCOOLER);
	std::for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the info command
 *
 * \param cooler	the cooler to query for info
 */
int	command_info(CoolerPrx cooler) {
	std::cout << "actual temperature: ";
	astro::Temperature	actualTemperature(cooler->getActualTemperature());
	astro::Temperature	setTemperature(cooler->getSetTemperature());
	std::cout << actualTemperature.celsius();
	std::cout << "°C" << std::endl;
	std::cout << "set temperature:    ";
	std::cout << setTemperature.celsius();
	std::cout << "°C" << std::endl;
	std::cout << "state:              ";
	std::cout << ((cooler->isOn()) ? "ON" : "OFF");
	std::cout << std::endl;
	if (cooler->hasDewHeater()) {
		std::cout << "dew heater:         ";
		std::cout << cooler->getDewHeater();
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Cooler callback implementation class
 */
class CoolerCallbackI : public CoolerCallback {
	void	timestamp() {
		astro::PrecisionTime	t;
		std::cout << t.toString("%T.%.03f: ");
	}
public:
	virtual void	updateCoolerInfo(const CoolerInfo& info,
				const Ice::Current& /* current */) {
		timestamp();
		astro::Temperature	actualTemperature(info.actualTemperature);
		astro::Temperature	setTemperature(info.setTemperature);
		std::cout << "actual=" << actualTemperature.celsius() << "°C";
		std::cout << ", ";
		std::cout << "set=" << setTemperature.celsius() << "°C";
		std::cout << ", ";
		if (info.on) {
			std::cout << "ON";
		} else {
			std::cout << "OFF";
		}
		std::cout << std::endl;
	}
	virtual void	updateSetTemperature(float settemperature,
				const Ice::Current& /* current */) {
		timestamp();
		astro::Temperature	temperature(settemperature);
		std::cout << "new set temperature: ";
		std::cout << temperature.celsius();
		std::cout << "°C" << std::endl;
	}
	virtual void	updateDewHeater(float dewheater,
				const Ice::Current& /* current */) {
		timestamp();
		std::cout << "dew heater: " << dewheater << std::endl;
	}
};

/**
 * \brief Signal handler
 */
void	signal_handler(int sig) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handling %d", sig);
}

/**
 * \brief Implementation of the monitor command
 *
 * \param cooler	the cooler to monitor
 */
int	command_monitor(CoolerPrx cooler) {
	// create and register the monitor callback
	CommunicatorSingleton::connect(cooler);
	CoolerCallbackI	*_callback = new CoolerCallbackI();
	Ice::ObjectPtr	callbackptr = _callback;
	Ice::Identity	ident = CommunicatorSingleton::add(callbackptr);
	cooler->registerCallback(ident);

	// install a signal handler
	struct sigaction	sa;
	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	// wait for an interrupt
	int	rc = 0;
	while (0 == (rc = sleep(60))) { }

	// unregister
	try {
		cooler->unregisterCallback(ident);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler callback %s unregistered",
			ident.name.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot unregister cooler "
			"callback %s , %s",
			ident.name.c_str(), x.what());
	}
	return EXIT_SUCCESS;
}

/**
 * \brief The main function for the snowcooler program
 *
 * \param argc	number of arguments
 * \param argv	argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowcooler");
	CommunicatorSingleton	comminicator(argc, argv);

	// parse the command line
	int	c;
	int	longindex;
	astro::ServerName	servername;
	putenv(strdup("POSIXLY_CORRECT=1"));
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts, &longindex)))
		switch(c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			break;
		}

	// we need at least one additional argument
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		return command_help(argv[0]);
	}

	// all other commands need a server name
	servername = astro::ServerName(command);
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	// handle thehlp command
	if (command == "help") {
		return command_help(argv[0]);
	}

	// all other commands need at least a devices proxy
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Devices"));
	DevicesPrx	devices = DevicesPrx::checkedCast(base);

	// handle the list command
	if (command == "list") {
		return command_list(devices);
	}

	// all other commands need a coler
	std::string	coolername = command;
	CoolerPrx	cooler = devices->getCooler(coolername);

	// no more arguments, handle it as an info command
	if (argc <= optind) {
		return command_info(cooler);
	}

	// we need a command
	command = std::string(argv[optind++]);
	if (command == "info") {
		return command_info(cooler);
	}
	if (command == "monitor") {
		return command_monitor(cooler);
	}
	if (command == "on") {
		cooler->setOn(true);
		return EXIT_SUCCESS;
	}
	if (command == "off") {
		cooler->setOn(false);
		return EXIT_SUCCESS;
	}

	// the next two commands required additional arguments
	if (optind >= argc) {
		throw std::runtime_error("need temperature");
	}
	std::string	argument(argv[optind++]);
	if (command == "set") {
		astro::Temperature	temperature(std::stod(argument),
						astro::Temperature::CELSIUS);
		cooler->setTemperature(temperature.temperature());
		return EXIT_SUCCESS;
	}
	if (command == "dewheater") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set dew heater %s",
			argument.c_str());
		cooler->setDewHeater(std::stod(argument));
		return EXIT_SUCCESS;
	}

	throw std::runtime_error("unknown command");
}

} // namepsace snowcooler
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowcooler::main>(argc, argv);
	CommunicatorSingleton::release();
	return rc;
}
