/*
 * snowguideport.cpp -- query or operate a guideport
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
namespace snowguideport {

static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ server ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> monitor <guideport>" << std::endl;
	std::cout << p << " [ options ] <server> active <guideport>" << std::endl;
	std::cout << p << " [ options ] <server> activate <RA+> <RA-> <DEC+> <DEC->" << std::endl;
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
	DeviceNameList  list = devices->getDevicelist(DevGUIDEPORT);
	std::for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

int	command_active(GuidePortPrx guideport) {
	uint8_t	act = guideport->active();
	if (act == 0) {
		std::cout << "(none)" << std::endl;
		return EXIT_SUCCESS;
	}
	if (act & snowstar::RAPLUS) {
		std::cout << "RA+ ";
	}
	if (act & snowstar::RAMINUS) {
		std::cout << "RA- ";
	}
	if (act & snowstar::DECPLUS) {
		std::cout << "DEC+ ";
	}
	if (act & snowstar::DECMINUS) {
		std::cout << "DEC- ";
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

class GuidePortCallbackI : public GuidePortCallback {
public:
	virtual void	activate(const snowstar::GuidePortActivation& act,
				const Ice::Current& /* current */) {
		std::cout << "RA+=" << act.raplus << " ";
		std::cout << "RA-=" << act.raminus << " ";
		std::cout << "DEC+=" << act.decplus << " ";
		std::cout << "DEC-=" << act.decminus << std::endl;
	}
};

void	signal_handler(int /* sig */) {
}

int	command_monitor(GuidePortPrx guideport) {
	GuidePortCallbackI	*_callback = new GuidePortCallbackI();
	Ice::ObjectPtr	callbackptr = _callback;
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callbackptr);
	guideport->ice_getConnection()->setAdapter(adapter.adapter());
	guideport->registerCallback(ident);
	signal(SIGINT, signal_handler);
	sleep(86400);
	guideport->unregisterCallback(ident);
	return EXIT_SUCCESS;
}

int	main(int argc, char *argv[]) {
	debug_set_ident("snowguideport");
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
		throw std::runtime_error("no guideport name");
	}
	std::string	guideportname(argv[optind++]);
	GuidePortPrx	guideport = devices->getGuidePort(guideportname);
	if (command == "active") {
		return command_active(guideport);
	}
	if (command == "monitor") {
		return command_monitor(guideport);
	}
	if (command == "activate") {
		// we need for more arguments
		if (optind + 4 < argc) {
			throw std::runtime_error("need for arguments to activate");
		}
		float	raplus = std::stod(argv[optind++]);
		float	raminus = std::stod(argv[optind++]);
		float	decplus = std::stod(argv[optind++]);
		float	decminus = std::stod(argv[optind++]);
		guideport->activate(raplus - raminus, decplus - decminus);
		return EXIT_SUCCESS;
	}

	throw std::runtime_error("unknown command");
}

} // namepsace snowguideport
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowguideport::main>(argc, argv);
	CommunicatorSingleton::release();
	return rc;
}
