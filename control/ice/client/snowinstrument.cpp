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
#include <IceConversions.h>
#include <IceDiscovery.h>
#include <CommonClientTasks.h>
#include <AstroFormat.h>
#include <AstroConfig.h>
#include <AstroProject.h>
#include <AstroIO.h>
#include <AstroDiscovery.h>
#include <iostream>
#include <algorithm>

namespace snowstar {
namespace app {
namespace snowinstrument {

bool	verbose = false;

/**
 * \brief Auxiliary functor class to display the list of instrument names
 */
class list_display {
	std::ostream&	_out;
public:
	list_display(std::ostream& out) : _out(out) { }
	void	operator()(const std::string& n) {
		_out << n << std::endl;
	}
};

/**
 * \brief Auxiliary operator to output the instrument components
 */
std::ostream&	operator<<(std::ostream& out,
			const InstrumentComponent& component) {
	out << instrumentcomponent2name(component.type) << "[";
	out << component.index << "] ";
	out << component.servicename << " ";
	out << component.deviceurl << " ";
	return out;
}

/**
 * \brief Auxiliary functor class to display the components of an instrument
 */
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

/**
 * \brief Auxiliary operator to output properties
 */
std::ostream&	operator<<(std::ostream& out,
	const InstrumentProperty& property) {
	out << property.instrumentname;
	out << ".";
	out << property.property;
	out << " = ";
	out << property.value;
	if (property.description.size() > 0) {
		out << " // ";
		out << property.description;
	}
	return out;
}

/**
 * \brief Auxiliary functor class to output a list of properties
 */
class list_property_display {
	std::ostream&	_out;
public:
	list_property_display(std::ostream& out) : _out(out) {
	}
	void	operator()(const InstrumentProperty& property) {
		_out << property << std::endl;
	}
};

/**
 * \brief Auxiliary class to display the the components of a list of instruments
 *
 * This operator class queries the instrument proxy for the list of components
 * of the instrument named in the argument, and displays all the components.
 */
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
		InstrumentPropertyList	plist = instrument->getProperties();
		std::for_each(plist.begin(), plist.end(),
			list_property_display(_out));
	}
};

/**
 * \brief Command to list instruments
 *
 * this command lists all the names of instruments available on the server.
 */
static int	list_command(InstrumentsPrx instruments) {
	// display the names
	InstrumentList	list = instruments->list();
	if (list.size() == 0) {
		std::cerr << "no instruments found" << std::endl;
		return EXIT_SUCCESS;
	}
	std::for_each(list.begin(), list.end(), list_display(std::cout));
	return EXIT_SUCCESS;
}

/**
 * \brief Command to list components of an instrument
 *
 * The argument strings are interpreted as instrument names, and the 
 * components of all of them are displayed
 */
static int	list_command(InstrumentsPrx instruments,
			const std::string& instrumentname) {
	list_instrument_display(std::cout, instruments)(instrumentname);
	return EXIT_SUCCESS;
}

/**
 * \brief command to add an instrument component 
 * 
 * This command requires four arguments: the instrument name, the type of
 * the component, the zeroconf service name and the device url on that server.
 */
static int	add_command(InstrumentsPrx instruments,
			const std::string& instrumentname,
			const std::vector<std::string>& arguments) {
	InstrumentComponent	component;
	if (arguments.size() < 3) {
		throw std::runtime_error("not enough arguments to add command");
	}
	component.instrumentname = instrumentname;
	component.type = name2instrumentcomponent(arguments[0]);
	component.servicename = arguments[1];
	component.deviceurl = arguments[2];
	InstrumentPrx	instrument = instruments->get(component.instrumentname);
	int	index = instrument->add(component);
	std::cout << "component added at index " << index << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Command to remove an instrument component
 *
 * This command requires three arguments: the name of the instrument,
 * the type of the component and the index of the component of this type
 */
static int	remove_command(InstrumentsPrx instruments,
			const std::string& instrumentname,
			const std::vector<std::string>& arguments) {
	if (arguments.size() < 1) {
		return EXIT_FAILURE;
	}
	InstrumentPrx	instrument = instruments->get(instrumentname);
	std::string	componentname = arguments[0];
	InstrumentComponentType	type;
	try {
		type = name2instrumentcomponent(componentname);
	} catch (const std::runtime_error& x) {
		std::string	propertyname = arguments[0];
		instrument->removeProperty(propertyname);
		return EXIT_SUCCESS;
	}
	unsigned int	index = 0;
	if (arguments.size() > 2) {
		index = std::stoi(arguments[1]);
	}
	instrument->remove(type, index);
	return EXIT_SUCCESS;
}

/**
 * \brief Command to destroy an instrument
 */
static int	destroy_command(InstrumentsPrx instruments,
			const std::string& instrumentname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying instrument '%s'",
		instrumentname.c_str());
	instruments->remove(instrumentname);
	return EXIT_SUCCESS;
}

/**
 * \brief 
 */
static void	short_usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << p << " [options] help" << std::endl;
	std::cout << p << " [options] <server> list" << std::endl;
	std::cout << p << " [options] <server> <INSTR> list" << std::endl;
	std::cout << p << " [options] <server> <INSTR> add <type> <service> <deviceurl>" << std::endl;
	std::cout << p << " [options] <server> <INSTR> remove <type> <index>" << std::endl;
	std::cout << p << " [options] <server> <INSTR> property <name> <value> <description>" << std::endl;
	std::cout << p << " [options] <server> <INSTR> remove <property>" << std::endl;
}

/**
 * \brief Usage function for the snowinstrument program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << "    " << p << " [options] command..."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug    increase debug level" << std::endl;
	std::cout << "  -h,--help     show this help and exit" << std::endl;
	std::cout << "  -v,--verbose  verbose mode" << std::endl;
	std::cout << std::endl;
	std::cout << "commands:" << std::endl;
	std::cout << std::endl;
	std::cout << "  help                              display this help message" << std::endl;
	std::cout << std::endl;
	std::cout << "  <server> list                     list instrument names" << std::endl;
	std::cout << std::endl;
	std::cout << "  <server> <INSTR> list             list components of instrument INSTR"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "  <server> <INSTR> add <type> <service> <deviceurl>" << std::endl;
	std::cout << "                                    add an instrument component"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "  <server> <INSTR> remove <type> <index>" << std::endl;
	std::cout << "                                    remove an instrument component"
		<< std::endl;
        std::cout << "                                    Valid component types are: Camera, CCD," << std::endl;
	std::cout << "                                    GuiderCCD, Cooler, GuiderPort, Focuser," << std::endl;
	std::cout << "                                    AdaptiveOptics, FilterWheel" << std::endl;
	std::cout << std::endl;
	std::cout << "  <server> <INSTR> property <name> [ <value> [ <description> ] ]" << std::endl;
	std::cout << "                                    add or change a property" << std::endl;
	std::cout << std::endl;
	std::cout << "  <server> <INSTR> remove <property>" << std::endl;
	std::cout << "                                    remove a property" << std::endl;
	std::cout << std::endl;
	std::cout << "  <server> <INSTR> destroy          destroy an instrument completely" << std::endl;
	std::cout << std::endl;
}

/**
 * \brief help command implementation
 */
static int	help_command(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

/**
 * \brief Command to display or modify a property
 */
static int	property_command(InstrumentsPrx instruments,
			const std::string& instrumentname,
			const std::vector<std::string>& arguments) {
	if (arguments.size() < 1) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	property = arguments[0];
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"property command %s.%s with %d arguments",
		instrumentname.c_str(), property.c_str(), arguments.size());
	InstrumentPrx	instrument = instruments->get(instrumentname);
	try {
		InstrumentProperty	p = instrument->getProperty(property);

		// the function depends on the number of arguments
		if (1 == arguments.size()) {
			std::cout << p << std::endl;
			return EXIT_SUCCESS;
		}

		// all other cases are update methods
		p.value = arguments[1];
		if (arguments.size() > 2) {
			p.description = arguments[2];
		}
		instrument->updateProperty(p);
		return EXIT_SUCCESS;
	} catch (NotFound x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"no property '%s', use add insetad", property.c_str());
	}

	// create a new property and add it
	InstrumentProperty	p;
	p.instrumentname = instrumentname;
	p.property = property;
	switch (arguments.size()) {
	case 3:
		p.value = arguments[2];
	case 2:
		p.value = arguments[1];
		break;
	case 1:
	case 0:
		throw std::runtime_error("not enough arguments");
	}
	instrument->addProperty(p);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new property added: %s.%s = %s",
		p.instrumentname.c_str(), p.property.c_str(),
		p.value.c_str());
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

/**
 * \brief Main function for the snowinstrument program
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowinstrument");
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
		std::cerr << "missing argument" << std::endl;
		short_usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	command = argv[optind++];
	if ("help" == command) {
		return help_command(argv[0]);
	}

	// if this was not the help command, then the string really was
	// the service name
	std::string	servicename = command;

	// resolve service name
	astro::discover::ServiceObject	serviceobject
		= IceDiscovery::discover(servicename);

	// connect to the server
	Ice::ObjectPrx	base = ic->stringToProxy(
				serviceobject.connect("Instruments"));
	InstrumentsPrx	instruments = InstrumentsPrx::checkedCast(base);

	// we need another argument, which we expect to be the service name
	if (optind >= argc) {
		std::cerr << "missing argument" << std::endl;
		short_usage(argv[0]);
		return EXIT_FAILURE;
	}
	command = argv[optind++];
	if ("list" == command) {
		return list_command(instruments);
	}

	// in this case the command was really the instrument name
	std::string	instrumentname = command;

	// and we need onther string, namely the command
	if (optind >= argc) {
		std::cerr << "no command" << std::endl;
		short_usage(argv[0]);
		return EXIT_FAILURE;
	}
	command = argv[optind++];

	// we pack the remaining arguments into a vector of strings
	std::vector<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}

	// execute command
	if (command == "list") {
		return list_command(instruments, instrumentname);
	}
	if (command == "add") {
		return add_command(instruments, instrumentname, arguments);
	}
	if (command == "remove") {
		return remove_command(instruments, instrumentname, arguments);
	}
	if (command == "property") {
		return property_command(instruments, instrumentname, arguments);
	}
	if (command == "destroy") {
		return destroy_command(instruments, instrumentname);
	}

	std::string	cause = astro::stringprintf("unknown command '%s'",
		command.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	short_usage(argv[0]);

	return EXIT_FAILURE;
}

} // namespace snowinstrument
} // namespace app
} // namespace snowtar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowinstrument::main>(argc, argv);
}
