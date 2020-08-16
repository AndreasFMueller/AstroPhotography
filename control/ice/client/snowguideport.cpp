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

/**
 * \brief Display a usage message
 *
 * \param progname 	name of the program to display in the message
 */
static void	usage(const std::string& progname) {
	std::cout << "Control a guideport in a snowstar server" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage:" << std::endl;
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << std::endl;
	std::cout << p << " [ options ] [ server ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> <guiderport> monitor"
		<< std::endl;
	std::cout << p << " [ options ] <server> <guiderport> active"
		<< std::endl;
	std::cout << p << " [ options ] <server> <guiderport> activate \\"
		<< std::endl;
	std::cout << "                                      <RA+> <RA-> <DEC+> <DEC->"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "If both RA+/- or DEC+/- are activated at the same time, "
		"which is not allowed," << std::endl;
	std::cout << "then the difference delta = X+ - X- is formed, and if "
		"the difference is" << std::endl;
	std::cout << "positive, the X+ terminal is activated for d seconds. "
		"If d is negative," << std::endl;
	std::cout << "then X- is activated for -d seconds." << std::endl;
	std::cout << std::endl;
}


static struct option	longopts[] = {
{ "debug",	no_argument,			NULL,		'd' },
{ "help",	no_argument,			NULL,		'h' },
{ NULL,		0,				NULL,		 0  }
};

/**
 * \brief Implementation of the help command
 *
 * The help command just displays the help message
 *
 * \param progname	the name of the program
 */
int	command_help(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the list command
 *
 * \param devices	a devices proxy to query for guideports
 */
int	command_list(DevicesPrx devices) {
	DeviceNameList  list = devices->getDevicelist(DevGUIDEPORT);
	std::for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the active command 
 *
 * The active command queries the current state of the guideport pins
 *
 * \param guideport	the guideport proxy to probe
 */
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

/**
 * \brief Monitor callback class
 *
 * The callback class displays activation changes reported by the
 * guide port
 */
class GuidePortCallbackI : public GuidePortCallback {
public:
	virtual void	activate(const snowstar::GuidePortActivation& act,
				const Ice::Current& /* current */) {
		astro::PrecisionTime	t;
		std::cout << t.toString("%T.%.04f: ");
		std::cout << "RA+=" << act.raplus << " ";
		std::cout << "RA-=" << act.raminus << " ";
		std::cout << "DEC+=" << act.decplus << " ";
		std::cout << "DEC-=" << act.decminus << std::endl;
	}
};

void	signal_handler(int /* sig */) {
}

/**
 * \brief Implementation of the monitor command
 *
 * The monitor command installs an instance of the GuidePortCallbackI
 * as a callback to the supplied guideport and displays any activation
 * changes that are sent.
 *
 * \param guideport	the guideport to monitor
 */
int	command_monitor(GuidePortPrx guideport) {
	// create and register the monitor callback
	CommunicatorSingleton::connect(guideport);
	GuidePortCallbackI	*_callback = new GuidePortCallbackI();
	Ice::ObjectPtr	callbackptr = _callback;
	Ice::Identity	ident = CommunicatorSingleton::add(callbackptr);
	guideport->registerCallback(ident);

	// install a signal handler
	struct sigaction	sa;
	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	// wait for an interrupt
	int	rc = 0;
	while (0 == (rc = sleep(60))) { }

	// unregister the callback
	guideport->unregisterCallback(ident);
	return EXIT_SUCCESS;
}

/**
 * \brief The main function for the guideport program
 *
 * \param argc	number of arguments
 * \param argv	the argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowguideport");
	CommunicatorSingleton	comminicator(argc, argv);

	// parse command line
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

	// check for an argument, which may be a command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		return command_help(argv[0]);
	}

	// if it wasn't a help command, then this was actually a server name
	servername = astro::ServerName(command);

	// if there are more arguments, then the next argument might again
	// be a command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	// handle the case that it is a help command again
	if (command == "help") {
		return command_help(argv[0]);
	}

	// if not, then we will certqinly need a devices proxy
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Devices"));
	DevicesPrx	devices = DevicesPrx::checkedCast(base);

	// handle the list command
	if (command == "list") {
		return command_list(devices);
	}

	// if this wasnt a list command, then the argument was actually
	// a guideport name, so we try to get the guideport
	std::string	guideportname = command;
	GuidePortPrx	guideport = devices->getGuidePort(guideportname);

	// if there are no more arguments, then handle this as a state
	// inquiry
	if (argc <= optind) {
		return command_active(guideport);
	}

	// otherwise, there will be more arguments
	command = std::string(argv[optind++]);
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
