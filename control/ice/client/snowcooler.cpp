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

static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> monitor <cooler>" << std::endl;
	std::cout << p << " [ options ] <server> info <cooler>" << std::endl;
	std::cout << p << " [ options ] <server> { on | off }<cooler>" << std::endl;
	std::cout << p << " [ options ] <server> set <cooler> <temperature>"
		<< std::endl;
	std::cout << p << " [ options ] <server> dewheater <cooler> <value>"
		<< std::endl;
	std::cout << std::endl;
}


static struct option	longopts[] = {
{ "debug",	no_argument,			NULL,		'd' },
{ "help",	no_argument,			NULL,		'h' },
{ NULL,		0,				NULL,		 0  }
};

int	command_help(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

int	command_list(DevicesPrx devices) {
	DeviceNameList  list = devices->getDevicelist(DevCOOLER);
	std::for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

int	command_info(CoolerPrx cooler) {
	std::cout << "actual temperature: ";
	astro::Temperature	actualTemperature(cooler->getActualTemperature());
	astro::Temperature	setTemperature(cooler->getSetTemperature());
	std::cout << actualTemperature.celsius();
	std::cout << "ºC" << std::endl;
	std::cout << "set temperature:    ";
	std::cout << setTemperature.celsius();
	std::cout << "ºC" << std::endl;
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

class CoolerCallbackI : public CoolerCallback {
	void	timestamp() {
	}
public:
	virtual void	updateCoolerInfo(const CoolerInfo& info,
				const Ice::Current& /* current */) {
		timestamp();
		astro::Temperature	actualTemperature(info.actualTemperature);
		astro::Temperature	setTemperature(info.setTemperature);
		std::cout << "actual=" << actualTemperature.celsius() << "ºC";
		std::cout << ", ";
		std::cout << "set=" << setTemperature.celsius() << "ºC";
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
		std::cout << "ºC" << std::endl;
	}
	virtual void	updateDewHeater(float dewheater,
				const Ice::Current& /* current */) {
		timestamp();
		std::cout << "dew heater: " << dewheater << std::endl;
	}
};

void	signal_handler(int /* sig */) {
}

int	command_monitor(CoolerPrx cooler) {
	CoolerCallbackI	*_callback = new CoolerCallbackI();
	Ice::ObjectPtr	callbackptr = _callback;
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callbackptr);
	cooler->ice_getConnection()->setAdapter(adapter.adapter());
	cooler->registerCallback(ident);
	signal(SIGINT, signal_handler);
	sleep(86400);
	cooler->unregisterCallback(ident);
	return EXIT_SUCCESS;
}

int	main(int argc, char *argv[]) {
	debug_set_ident("snowcooler");
	CommunicatorSingleton	comminicator(argc, argv);

	int	c;
	int	longindex;
	astro::ServerName	servername;
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
		return command_help(argv[0]);
	}

	servername = astro::ServerName(command);

	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	if (command == "help") {
		return command_help(argv[0]);
	}

	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Devices"));
	DevicesPrx	devices = DevicesPrx::checkedCast(base);

	if (command == "list") {
		return command_list(devices);
	}

	if (argc <= optind) {
		throw std::runtime_error("no cooler name");
	}
	std::string	coolername(argv[optind++]);
	CoolerPrx	cooler = devices->getCooler(coolername);
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
	// the next two commands required an argument
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
