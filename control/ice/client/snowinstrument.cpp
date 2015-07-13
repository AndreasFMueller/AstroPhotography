/*
 * snowinstrument.cpp -- client to manage instruments
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <includes.h>
#include <AstroConfig.h>
#include <tasks.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>
#include <AstroFormat.h>
#include <AstroConfig.h>
#include <AstroProject.h>
#include <AstroIO.h>
#include <ServiceDiscovery.h>
#include <iostream>
#include <algorithm>

namespace snowstar {
namespace app {
namespace snowinstrument {

bool	verbose = false;

static InstrumentComponentType	string2type(const std::string& componenttype) {
	if (componenttype == "CCD") {
		return InstrumentCCD;
	}
	if (componenttype == "GuiderCCD") {
		return InstrumentGuiderCCD;
	}
	if (componenttype == "Cooler") {
		return InstrumentCooler;
	}
	if (componenttype == "GuiderPort") {
		return InstrumentGuiderPort;
	}
	if (componenttype == "Focuser") {
		return InstrumentFocuser;
	}
	if (componenttype == "AdaptiveOptics") {
		return InstrumentAdaptiveOptics;
	}
	throw std::runtime_error("unknown component type");
}

static std::string	type2string(InstrumentComponentType type) {
	switch (type) {
		case InstrumentCCD:
			return std::string("CCD");
		case InstrumentGuiderCCD:
			return std::string("GuiderCCD");
		case InstrumentCooler:
			return std::string("Cooler");
		case InstrumentGuiderPort:
			return std::string("GuiderPort");
		case InstrumentFocuser:
			return std::string("Focuser");
		case InstrumentAdaptiveOptics:
			return std::string("AdaptiveOptics");
	}
	throw std::runtime_error("invalid type code");
}

class list_display {
	std::ostream&	_out;
public:
	list_display(std::ostream& out) : _out(out) { }
	void	operator()(const std::string& n) {
		_out << n << std::endl;
	}
};

std::ostream&	operator<<(std::ostream& out,
			const InstrumentComponent& component) {
	out << type2string(component.type) << "[";
	out << component.index << "] ";
	out << component.servicename << " ";
	out << component.deviceurl << " ";
	return out;
}

class list_component_display {
	std::string	_instrumentname;
	std::ostream&	_out;
public:
	list_component_display(const std::string& instrumentname,
		std::ostream& out)
		: _instrumentname(instrumentname), _out(out) {
	}
	void	operator()(const InstrumentComponent& component) {
		_out << _instrumentname << " " << component << std::endl;
	}
};

class list_instrument_display {
	std::ostream&	_out;
	InstrumentsPrx	_instruments;
public:
	list_instrument_display(std::ostream& out, InstrumentsPrx instruments)
		: _out(out), _instruments(instruments) {
	}
	void	operator()(const std::string& instrumentname) {
		InstrumentPrx	instrument = _instruments->get(instrumentname);
		InstrumentComponentList	list = instrument->list();
		std::for_each(list.begin(), list.end(),
			list_component_display(instrumentname, _out));
	}
};

static int	list_command(InstrumentsPrx instruments,
			const std::vector<std::string>& arguments) {
	if (arguments.size() > 0) {
		std::for_each(arguments.begin(), arguments.end(),
			list_instrument_display(std::cout, instruments));
		return EXIT_SUCCESS;
	}
	InstrumentList	list = instruments->list();
	if (list.size() == 0) {
		std::cerr << "no instruments found" << std::endl;
		return EXIT_SUCCESS;
	}
	std::for_each(list.begin(), list.end(), list_display(std::cout));
	return EXIT_SUCCESS;
}

static int	add_command(InstrumentsPrx instruments,
			const std::vector<std::string>& arguments) {
	InstrumentComponent	component;
	if (arguments.size() < 4) {
		throw std::runtime_error("not enough arguments to add command");
	}
	component.instrumentname = arguments[0];
	component.type = string2type(arguments[1]);
	component.servicename = arguments[2];
	component.deviceurl = arguments[3];
	InstrumentPrx	instrument = instruments->get(component.instrumentname);
	int	index = instrument->add(component);
	std::cout << "component added at index " << index << std::endl;
	return EXIT_SUCCESS;
}

static int	remove_command(InstrumentsPrx instruments,
			const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		return EXIT_FAILURE;
	}
	std::string	instrumentname = arguments[0];
	InstrumentComponentType	type = string2type(arguments[1]);
	int	index = std::stoi(arguments[2]);
	InstrumentPrx	instrument = instruments->get(instrumentname);
	instrument->remove(type, index);
	return EXIT_SUCCESS;
}

/**
 * \brief Usage function for the snowtask program
 */
void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << "    " << p << " [options] servicename" << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          show this help and exit" << std::endl;
	std::cout << " -v,--verbose       verbose mode" << std::endl;
}

/**
 * \brief Options for the snowtask program
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' }, /*  1 */
{ "help",	no_argument,		NULL,		'h' }, /*  2 */
{ "verbose",	no_argument,		NULL,		'v' }, /*  3 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function for the snowtask program
 */
int	main(int argc, char *argv[]) {
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
		}

	// the next argument is the name of the service
	if (optind >= argc) {
		std::cerr << "missing service name argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	name = argv[optind++];

	// get command information
	if (optind >= argc) {
		std::cerr << "no command" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	command(argv[optind++]);
	std::vector<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}

	// resolve service name
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service: %s", name.c_str());
	astro::discover::ServiceDiscoveryPtr	discovery
		= astro::discover::ServiceDiscovery::get();
	discovery->start();
	astro::discover::ServiceKey	key = discovery->waitfor(name);
	astro::discover::ServiceObject	serviceobject = discovery->find(key);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "host: %s, port: %d",
		serviceobject.host().c_str(), serviceobject.port());

	// connect to the server
	Ice::ObjectPrx	base = ic->stringToProxy(serviceobject.connect("Instruments"));
	InstrumentsPrx	instruments = InstrumentsPrx::checkedCast(base);

	// execute command
	if (command == "list") {
		return list_command(instruments, arguments);
	}
	if (command == "add") {
		return add_command(instruments, arguments);
	}
	if (command == "remove") {
		return remove_command(instruments, arguments);
	}

	return EXIT_FAILURE;
}

} // namespace snowtask
} // namespace app
} // namespace snowtar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowinstrument::main>(argc, argv);
}
