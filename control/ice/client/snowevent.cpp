/*
 * snowevent.cpp -- event client for the snowstar server
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <IceDiscovery.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <includes.h>

namespace snowstar {
namespace app {
namespace snowevent {

bool verbose = false;

/**
 * \brief Event display class
 */
class EventDisplay {
	std::ostream&	_out;
	bool	_verbose;
public:
	EventDisplay(std::ostream& out, bool verbose)
		: _out(out), _verbose(verbose) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "verbose: %s (currently unused",
			(_verbose) ? "yes" : "no");
	}
	void	operator()(const Event& event);
};

void	EventDisplay::operator()(const Event& event) {
	struct timeval	t = converttimeval(event.timeago);
	char	buffer[30];
	struct tm	*tmp = localtime(&t.tv_sec);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tmp);
	_out << buffer;
	_out << astro::stringprintf(".%03d", (t.tv_usec / 1000));
	_out << " ";
	_out << astro::stringprintf("%-6.6s", event.subsystem.c_str());
	_out << " ";
	_out << event.message << std::endl;
}

/**
 * \brief Event monitor
 */
class EventMonitorI : public EventMonitor, public CommonMonitor {
	EventDisplay	out;
public:
	EventMonitorI() : out(std::cout, verbose) {
	}
	void	update(const Event& event, const Ice::Current& /* current */) {
		out(event);
	}
	void	stop(const Ice::Current& /* current */) {
		complete(true);
	}
};

EventMonitorI	*eventmonitor;

void	signal_handler(int /* sig */) {
	if (eventmonitor) {
		eventmonitor->complete(true);
	}
}

/**
 * \brief Usage function for the snowinstrument program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << "    " << p << " [options] help"
		<< std::endl;
	std::cout << "    " << p << " [options] server show"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug    increase debug level" << std::endl;
	std::cout << "  -h,--help     show this help and exit" << std::endl;
	std::cout << "  -v,--verbose  verbose mode" << std::endl;
	std::cout << std::endl;
	std::cout << "commands:" << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
}

/**
 * \brief help command implementation
 */
static int	help_command(const char *p) {
	usage(p);
	return EXIT_SUCCESS;
}

/**
 * \brief Options for the snowinstrument program
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' }, /*  1 */
{ "help",	no_argument,		NULL,		'h' }, /*  2 */
{ "verbose",	no_argument,		NULL,		'v' }, /*  3 */
{ NULL,		0,			NULL,		0   }
};

int	show_command(EventHandlerPrx events) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "executing show command");
	eventlist	el = events->eventsBetween(100000, -10);
	if (0 == el.size()) {
		std::cerr << "no events found" << std::endl;
		return EXIT_SUCCESS;
	}
	EventDisplay	out(std::cout, verbose);
	std::for_each(el.begin(), el.end(), out);
	return EXIT_SUCCESS;
}

int	monitor_command(EventHandlerPrx events) {
	// create an instance of the callback
	eventmonitor = new EventMonitorI();
	Ice::ObjectPtr	callback = eventmonitor;

	// register the callback with the adapter
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	events->ice_getConnection()->setAdapter(adapter.adapter());

	// install the signal handler
	signal(SIGINT, signal_handler);

	// register the callback with the server
	events->registerMonitor(ident);
	eventmonitor->wait();
	events->unregisterMonitor(ident);
	return EXIT_SUCCESS;
}

/**
 * \brief Main function for the snowinstrument program
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowevent");
	CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	debugthreads = true;

	// parse command line options
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?v",
			longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'v':
			verbose = true;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// the next argument is the name of the service
	if (optind >= argc) {
		std::cerr << "missing argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	command = argv[optind++];
	if ("help" == command) {
		return help_command(argv[0]);
	}

	// if this was not the help command, then the string really was
	// the service name
	std::string	servicename = command;
	if (optind >= argc) {
		std::cerr << "missing sevice argument" << std::endl;
		return EXIT_FAILURE;
	}
	command = std::string(argv[optind++]);

	// resolve service name
	astro::discover::ServiceObject	serviceobject
		= IceDiscovery::discover(servicename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connected to service '%s'",
		servicename.c_str());

	// connect to the server
	Ice::ObjectPrx	base = ic->stringToProxy(
				serviceobject.connect("Events"));
	if (!base) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no event server found");
		return EXIT_FAILURE;
	}
	EventHandlerPrx	events = EventHandlerPrx::checkedCast(base);
	if (!events) {
		debug(LOG_ERR, DEBUG_LOG, 0, "have not EventHandler proxy");
		return EXIT_FAILURE;
	}

	// now interpret the command
	if ("show" == command) {
		return show_command(events);
	}
	if ("monitor" == command) {
		return monitor_command(events);
	}

	std::string	cause = astro::stringprintf("unknown command '%s'",
		command.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());

	return EXIT_FAILURE;
}

} // namespace snowevent
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowevent::main>(argc,
			argv);
	snowstar::CommunicatorSingleton::release();
	return rc;
}
