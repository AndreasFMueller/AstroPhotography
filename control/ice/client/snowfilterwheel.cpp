/*
 * snowfilterwheel.cpp -- query or operate a filterwheel
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
namespace snowfilterwheel {

static void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ <server> ] help" << std::endl;
	std::cout << p << " [ options ] <server> list" << std::endl;
	std::cout << p << " [ options ] <server> monitor <filterwheel>" << std::endl;
	std::cout << p << " [ options ] <server> info <filterwheel>" << std::endl;
	std::cout << p << " [ options ] <server> select <filterwheel> <filter>"
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
	DeviceNameList  list = devices->getDevicelist(DevFILTERWHEEL);
	std::for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

int	command_info(FilterWheelPrx filterwheel) {
	int	n = filterwheel->nFilters();
	std::cout << "number of filters: " << n << std::endl;
	for (int i = 0; i < n; i++) {
		std::cout << "filter[" << i << "] = ";
		std::cout << filterwheel->filterName(i);
		std::cout << std::endl;
	}
	std::cout << "current state:     ";
	switch (filterwheel->getState()) {
	case snowstar::FwIDLE:
		std::cout << "idle";
		break;
	case snowstar::FwMOVING:
		std::cout << "moving";
		break;
	case snowstar::FwUNKNOWN:
		std::cout << "unknown";
		break;
	}
	std::cout << std::endl;
	if (snowstar::FwIDLE == filterwheel->getState()) {
		std::cout << "position:          ";
		std::cout << filterwheel->currentPosition();
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

class FilterWheelCallbackI : public FilterWheelCallback {
	void	timestamp() {
		astro::PrecisionTime	t;
		std::cout << t.toString("%T.%.03f: ");
	}
public:
	void	state(snowstar::FilterwheelState s,
			const Ice::Current& /* current */) {
		std::cout << "state change: ";
		switch (s) {
		case snowstar::FwIDLE:
			std::cout << "idle";
			break;
		case snowstar::FwMOVING:
			std::cout << "moving";
			break;
		case snowstar::FwUNKNOWN:
			std::cout << "unknown";
			break;
		}
		std::cout << std::endl;
	}
	void	position(int filter, const Ice::Current& /* current */) {
		timestamp();
		std::cout << "filter change: " << filter << std::endl;
	}
	void	stop(const Ice::Current& /* current */) {
		timestamp();
		std::cout << "stop" << std::endl;
		exit(EXIT_SUCCESS);
	}
};

void	signal_handler(int /* sig */) {
}

int	command_monitor(FilterWheelPrx filterwheel) {
	CommunicatorSingleton::connect(filterwheel);
	FilterWheelCallbackI	*_callback = new FilterWheelCallbackI();
	Ice::ObjectPtr	callbackptr = _callback;
	Ice::Identity	ident = CommunicatorSingleton::add(callbackptr);
	filterwheel->registerCallback(ident);
	signal(SIGINT, signal_handler);
	sleep(86400);
	filterwheel->unregisterCallback(ident);
	return EXIT_SUCCESS;
}

int	main(int argc, char *argv[]) {
	debug_set_ident("snowfilterwheel");
	CommunicatorSingleton	comminicator(argc, argv);

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
		throw std::runtime_error("no filterwheel name");
	}
	std::string	filterwheelname(argv[optind++]);
	FilterWheelPrx	filterwheel = devices->getFilterWheel(filterwheelname);
	if (command == "info") {
		return command_info(filterwheel);
	}
	if (command == "monitor") {
		return command_monitor(filterwheel);
	}
	// the next two commands required an argument
	if (optind >= argc) {
		throw std::runtime_error("need temperature");
	}
	std::string	argument(argv[optind++]);
	if (command == "select") {
		try {
			filterwheel->select(std::stoi(argument));
		} catch (...) {
			filterwheel->selectName(argument);
		}
		return EXIT_SUCCESS;
	}

	throw std::runtime_error("unknown command");
}

} // namepsace snowfilterwheel
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowfilterwheel::main>(argc, argv);
	CommunicatorSingleton::release();
	return rc;
}
