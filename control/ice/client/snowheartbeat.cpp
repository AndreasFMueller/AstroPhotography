/*
 * snowheartbeat.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <includes.h>
#include <AstroConfig.h>
#include <types.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>
#include <AstroFormat.h>
#include <getopt.h>
#include <iomanip>

namespace snowstar {
namespace app {
namespace heartbeat {

bool	completed = false;

time_t	lastupdate;

void	signal_handler(int /* sig */) {
	completed = true;
}

class HeartbeatMonitorI : public HeartbeatMonitor {
	astro::Timer	_timer;
public:
	static float	_interval;
	HeartbeatMonitorI() {
		_timer.start();
	}
	virtual ~HeartbeatMonitorI() {
	}
	virtual void	beat(const int sequence_number,
			const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update received: %d",
			sequence_number);
		time(&lastupdate);
		_timer.end();
		std::cout << _timer.timestamp(3);
		std::cout << " delta = ";
		std::cout << astro::stringprintf("%5.3f", _timer.elapsed());
		std::cout << ": seqno = ";
		std::cout << sequence_number;
		std::cout << std::endl;
		_timer.start();
	}
	virtual void	interval(const float interval,
			const Ice::Current& /* current */) {
		_interval = interval;
		std::cout << "interval: " << _interval << std::endl;
	}
	virtual void	stop(const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
		completed = true;
	}
};

float	HeartbeatMonitorI::_interval = 0;

void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <service> monitor" << std::endl;
	std::cout << p << " [ options ] <service> pause" << std::endl;
	std::cout << p << " [ options ] <service> resume" << std::endl;
	std::cout << p << " [ options ] <service> interval <interval>"
			 << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug                increase debug level";
	std::cout << std::endl;
	std::cout << "  -h,-?,--help              display this help message "
		"and exit";
	std::cout << std::endl;
}

int	command_help(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,		'd' },
{ "help",		no_argument,		NULL,		'h' },
{ NULL,			0,			NULL,		 0  }
};

static int	monitor(DaemonPrx daemon) {
	// get the interval
	HeartbeatMonitorI::_interval = daemon->heartbeatInterval();
	std::cout << "interval: " << HeartbeatMonitorI::_interval << std::endl;

	// create the heartbeat monitor
	HeartbeatMonitorI	*heartbeatmonitor = new HeartbeatMonitorI();
        Ice::ObjectPtr  monitor = heartbeatmonitor;

	// get the adapter and construct the identity
        Ice::Identity   ident = CommunicatorSingleton::add(monitor);

	// set the last update timer
	time(&lastupdate);

        // register the monitor
        daemon->registerHeartbeatMonitor(ident);

	// wait for the monitor to complete
        signal(SIGINT, signal_handler);
        while (!completed) {
                sleep(1);
		time_t	now;
		time(&now);
		if ((HeartbeatMonitorI::_interval > 0) && ((now - lastupdate) > (2 * HeartbeatMonitorI::_interval))) {
			std::cerr << "missed heartbeat: last ";
			std::cerr << (now - lastupdate);
			std::cerr << " seconds ago";
			std::cerr << std::endl;
			try {
				time(&lastupdate);
        			ident = CommunicatorSingleton::add(monitor);
				std::cerr << "reregistered" << std::endl;
				debug(LOG_DEBUG, DEBUG_LOG, 0, "interval: %.3f",
					HeartbeatMonitorI::_interval);
			} catch (const std::exception& x) {
				std::cerr << "cannot reconnect: ";
				std::cerr << x.what();
				std::cerr << std::endl;
			}
		}
        }

	// unregister the monitor
	try {
		daemon->unregisterHeartbeatMonitor(ident);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "could not unregister: %s",
			x.what());
	}
	return EXIT_SUCCESS;
}

int	main(int argc, char *argv[]) {
	debug_set_ident("snowheartbeat");
	CommunicatorSingleton   cs(argc, argv);
	Ice::CommunicatorPtr    ic = CommunicatorSingleton::get();
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "de:fh?i:p:R:D:l:L:",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

	// next argument is help or servername
	if (argc <= optind) {
		throw std::runtime_error("server or command name missing");
	}
	std::string	command(argv[optind++]);
	if (command == "help") {
		command_help(argv[0]);
	}
	astro::ServerName	servername = astro::ServerName(command);

	// get the command
	if (optind >= argc) {
		std::cerr << "command missing" << std::endl;
		return EXIT_FAILURE;
	}
	command = std::string(argv[optind++]);

	// get the heartbeat interface
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Daemon"));
	DaemonPrx	daemon = DaemonPrx::checkedCast(base);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "daemon proxy created");
	CommunicatorSingleton::connect(daemon);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "daemon connected");

	if (command == "monitor") {
		return monitor(daemon);
	}

	if (command == "pause") {
		daemon->pauseHeartbeat();
		return EXIT_SUCCESS;
	}

	if (command == "resume") {
		daemon->resumeHeartbeat();
		return EXIT_SUCCESS;
	}

	if (command == "interval") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "processing interval command");
		if (optind >= argc) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "get interval");
			try {
				std::cout << "interval: ";
				std::cout << daemon->heartbeatInterval();
				std::cout << std::endl;
			} catch (const std::exception& x) {
				debug(LOG_ERR, DEBUG_LOG, 0,
					"cannot get interval: %s", x.what());
			}
		} else {
			float	interval = std::stod(argv[optind++]);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "set interval to %.3f",
				interval);
			try {
				daemon->setHeartbeatInterval(interval);
			} catch (const std::exception& x) {
				debug(LOG_ERR, DEBUG_LOG, 0,
					"cannot set interval %.3f: %s",
					interval, x.what());
			}
		} 
		return EXIT_SUCCESS;
	}

	std::cerr << "unknown command: " << command << std::endl;
	return EXIT_FAILURE;
}

} // namespace heartbeat
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::heartbeat::main>(argc,
			argv);
	snowstar::CommunicatorSingleton::release();
	return rc;
}
