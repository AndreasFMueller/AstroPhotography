/*
 * snowfocuser.cpp -- query or operate a focuser
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
namespace snowfocuser {

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
	std::cout << p << " [ options ] <server> <focuser> monitor"
		<< std::endl;
	std::cout << p << " [ options ] <server> <focuser> [ get ]"
		<< std::endl;
	std::cout << p << " [ options ] <server> <focuser> info" << std::endl;
	std::cout << p << " [ options ] <server> <focuser> set <position>"
		<< std::endl;
	std::cout << p << " [ options ] <server> <focuser> dewheater <value>"
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
 * \param devices	the devices proxy to query for focusers
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
 * \param focuser	the focuser to query for info
 */
int	command_info(FocuserPrx focuser) {
	std::cout << "name:      " << focuser->getName() << std::endl;
	std::cout << "minimum:   " << focuser->min() << std::endl;
	std::cout << "maximum:   " << focuser->max() << std::endl;
	std::cout << "current:   " << focuser->current() << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the info command
 *
 * \param focuser	the focuser to query for info
 */
int	command_get(FocuserPrx focuser) {
	std::cout << focuser->current() << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Focuser callback implementation class
 */
class FocuserCallbackI : public FocuserCallback {
	void	timestamp() {
		astro::PrecisionTime	t;
		std::cout << t.toString("%T.%.03f: ");
	}
public:
	virtual void	movement(Ice::Long fromposition, Ice::Long toposition,
				const Ice::Current& /* current */) {
		timestamp();
		std::cout << "from=" << fromposition;
		std::cout << ", to=" << toposition;
		std::cout << std::endl;
	}
	virtual void	info(Ice::Long currentposition, bool on_target,
				const Ice::Current& /* current */) {
		timestamp();
		std::cout << "current=" << currentposition;
		if (on_target) {
			std::cout << "  on target";
		}
		std::cout << std::endl;
	}
	virtual void	stop(const Ice::Current& /* current */) {
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
 * \param focuser	the focuser to monitor
 */
int	command_monitor(FocuserPrx focuser) {
	// create and register the monitor callback
	CommunicatorSingleton::connect(focuser);
	FocuserCallbackI	*_callback = new FocuserCallbackI();
	Ice::ObjectPtr	callbackptr = _callback;
	Ice::Identity	ident = CommunicatorSingleton::add(callbackptr);
	focuser->registerCallback(ident);

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
		focuser->unregisterCallback(ident);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser monitor %s unregistered",
			ident.name.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot unregister focuser "
			"callback %s: %s", ident.name.c_str(), x.what());
	}
	return EXIT_SUCCESS;
}

/**
 * \brief The main function for the snowfocuser program
 *
 * \param argc	number of arguments
 * \param argv	argument vector
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowfocuser");
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
	std::string	focusername = command;
	FocuserPrx	focuser = devices->getFocuser(focusername);

	// no more arguments, handle it as an info command
	if (argc <= optind) {
		return command_get(focuser);
	}

	// we need a command
	command = std::string(argv[optind++]);
	if (command == "get") {
		return command_get(focuser);
	}
	if (command == "info") {
		return command_info(focuser);
	}
	if (command == "monitor") {
		return command_monitor(focuser);
	}

	// the next two commands required additional arguments
	if (optind >= argc) {
		throw std::runtime_error("need position");
	}
	std::string	argument(argv[optind++]);
	if (command == "set") {
		long	position = std::stoi(argument);
		focuser->set(position);
		return EXIT_SUCCESS;
	}

	throw std::runtime_error("unknown command");
}

} // namepsace snowfocuser
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowfocuser::main>(argc, argv);
	CommunicatorSingleton::release();
	return rc;
}
