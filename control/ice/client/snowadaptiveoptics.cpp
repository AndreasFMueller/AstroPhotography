/*
 * snowadaptiveoptics.cpp -- query or position adaptiveoptics
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <cstdlib>
#include <device.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <includes.h>
#include <AstroConfig.h>
#include <IceConversions.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace adaptiveoptics {

bool	await_completion = false;
bool	decimal = false;

/**
 * \brief Usage function for the snowadaptiveoptics function
 */
static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") +  path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> get <adaptiveoptics>" << std::endl;
	std::cout << p << " [ options ] <server> set <adaptiveoptics> <x> <y>" << std::endl;
	std::cout << p << " [ options ] <server> center <adaptiveoptics>" << std::endl;
	std::cout << p << " [ options ] <server> monitor <adaptiveoptics>" << std::endl;
	std::cout << std::endl;
	std::cout << "get help about the snowadaptiveoptics command, list "
		"adaptiveoptics," << std::endl;
	std::cout << "get and set the adaptiveoptics coordinates or center it"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          display this help message"
		<< std::endl;
	std::cout << std::endl;
}

/**
 * \brief array of options
 */
static struct option    longopts[] = {
{ "debug",	no_argument,			NULL,	'd' }, /* 1 */
{ "help",	no_argument,			NULL,	'h' }, /* 3 */
{ NULL,		0,				NULL,	0   }
};

/**
 * \brief Help command implementation
 */
int	command_help(const char *progname) {
	usage(progname);
	std::cout << "The snowadaptiveoptics command understands the following "
		"subcommands:" << std::endl;
	std::cout << std::endl;
	std::cout << "help" << std::endl;
	std::cout << "    Display this help" << std::endl;
	std::cout << std::endl;
	std::cout << "list" << std::endl;
	std::cout << "    List all adaptiveopticss available from the server"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "location <adaptiveoptics>" << std::endl;
	std::cout << "    Get the location of the adaptiveoptics" << std::endl;
	std::cout << std::endl;
	std::cout << "get <adaptiveoptics>" << std::endl;
	std::cout << "    Get current coordinates" << std::endl;
	std::cout << std::endl;
	std::cout << "set <adaptiveoptics> <x> <y>" << std::endl;
	std::cout << "    Move the adaptiveoptics to the specified coordinates"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "monitor <adaptiveoptics>" << std::endl;
	std::cout << "    monitor state changes and position changes on this adaptiveoptics." << std::endl;
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

int	command_list(DevicesPrx devices) {
	DeviceNameList	list = devices->getDevicelist(DevAO);
	std::for_each(list.begin(), list.end(), 
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

class AdaptiveOpticsCallbackI : public AdaptiveOpticsCallback {
	void	timestamp() {
		astro::PrecisionTime	t;
		std::cout << t.toString("%T.%.03f:  ");
	}
public:
	virtual void	stop(const Ice::Current& /* current */) {
		timestamp();
		std::cout << "stop" << std::endl;
		exit(EXIT_SUCCESS);
	}
	virtual void	point(const Point& newposition,
				const Ice::Current& /* current */) {
		timestamp();
		std::cout << convert(newposition).toString() << std::endl;
	}
};

void    signal_handler(int /* sig */) {
	
}

/**
 * \brief Monitor the adaptiveoptics
 *
 * \param adaptiveoptics	the adaptive optics proxy
 */
int	command_monitor(AdaptiveOpticsPrx adaptiveoptics) {
	CommunicatorSingleton::connect(adaptiveoptics);

	// create a callback object
	AdaptiveOpticsCallbackI	*_callback = new AdaptiveOpticsCallbackI();

	// register the callback with the adaptiveoptics
	Ice::ObjectPtr  callbackptr = _callback;
	Ice::Identity	ident = CommunicatorSingleton::add(callbackptr);
        adaptiveoptics->registerCallback(ident);

	// install a signal handler
	signal(SIGINT, signal_handler);
	
	// wait indefinitely
	sleep(86400);
	adaptiveoptics->unregisterCallback(ident);
	return EXIT_SUCCESS;
}

/**
 * \brief main function 
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowadaptiveoptics");
	CommunicatorSingleton	communicator(argc, argv);
	
	int	c;
	int	longindex;
	astro::ServerName	servername;
	putenv(strdup("POSIXLY_CORRECT=1"));
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
		default:
			throw std::runtime_error("unknown option");
		}

	// next comes the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		return command_help(argv[0]);
	}

	servername = astro::ServerName(command);

	// next argument must be the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	if (command == "help") {
		return command_help(argv[0]);
	}

	// we need a remote device proxy for all other commands
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Devices"));
	DevicesPrx	devices = DevicesPrx::checkedCast(base);

	// handle the list command
	if (command == "list") {
		return command_list(devices);
	}

	// for the other commands we need the adaptiveoptics name
	if (argc <= optind) {
		throw std::runtime_error("no adaptiveoptics name");
	}
	std::string	adaptiveopticsname(argv[optind++]);

	// get a proxy for the adaptiveoptics
	AdaptiveOpticsPrx	adaptiveoptics
		= devices->getAdaptiveOptics(adaptiveopticsname);

	// get command
	if (command == "get") {
		snowstar::Point	point = adaptiveoptics->get();
		std::cout << point.x << ", " << point.y << std::endl;
		return EXIT_SUCCESS;
	}
	if (command == "monitor") {
		return command_monitor(adaptiveoptics);
	}
	if (command == "center") {
		adaptiveoptics->center();
		return EXIT_SUCCESS;
	}
	if (command == "monitor") {
		return command_monitor(adaptiveoptics);
	}

	// two more arguments are angles
	if (command == "set") {
		if (argc < (optind + 2)) {
			throw std::runtime_error("missing coordinate arguments");
		}
		snowstar::Point	point;
		point.x = std::stod(argv[optind++]);
		point.y = std::stod(argv[optind++]);
		adaptiveoptics->set(point);
		return EXIT_SUCCESS;
	}

	// if we get here, then an unknown command was given
	throw std::runtime_error("unknown command");
}

} // namespace adaptiveoptics
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::adaptiveoptics::main>(argc,
			argv);
	CommunicatorSingleton::release();
	return rc;
}
