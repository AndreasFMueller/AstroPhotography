/*
 * snowgateway.cpp
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

namespace snowstar {
namespace app {
namespace gateway {

bool	completed = false;

void	signal_handler(int /* sig */) {
	completed = true;
}

class StatusUpdateMonitorI : public StatusUpdateMonitor {
public:
	StatusUpdateMonitorI() {
	}
	void	update(const StatusUpdate& statusupdate,
			const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update received");
	}
	void	stop(const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
		completed = true;
	}
};

void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <service> help" << std::endl;
	std::cout << p << " [ options ] <service> send" << std::endl;
	std::cout << p << " [ options ] <service> monitor" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug          increase debug level" << std::endl;
	std::cout << "  -h,-?,--help        display this help message and exit" << std::endl;
}

int	command_help(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

int	command_monitor(GatewayPrx gateway) {
	Ice::ObjectPtr	monitor = new StatusUpdateMonitorI();
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(monitor);
	gateway->ice_getConnection()->setAdapter(adapter.adapter());
	gateway->registerMonitor(ident);

	signal(SIGINT, signal_handler);
	while (!completed) {
		sleep(1);
	}

	gateway->unregisterMonitor(ident);
	return EXIT_SUCCESS;
}

int	command_send(GatewayPrx gateway) {
	snowstar::StatusUpdate	update;
	gateway->send(update);
	return EXIT_SUCCESS;
}


static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,		'd' },
{ "help",		no_argument,		NULL,		'h' },
{ NULL,			0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	debug_set_ident("snowgateway");
	CommunicatorSingleton   cs(argc, argv);
	Ice::CommunicatorPtr    ic = CommunicatorSingleton::get();

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
			break;
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
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);
	if (command == "help") {
		return command_help(argv[0]);
	}

	// get the gateway interface
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Gateway"));
        GatewayPrx	gateway = GatewayPrx::checkedCast(base);

	if (command == "monitor") {
		return command_monitor(gateway);
	}

	if (command == "send") {
		return command_send(gateway);
	}

	return EXIT_SUCCESS;
}


} // namespace gateway
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::gateway::main>(argc,
			argv);
	snowstar::CommunicatorSingleton::release();
	return rc;
}
