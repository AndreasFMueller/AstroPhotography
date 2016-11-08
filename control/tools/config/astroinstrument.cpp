/*
 * astroinstrument.cpp -- astrophotography instrument construction
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroUtils.h>
#include <AstroPersistence.h>
#include <AstroDiscovery.h>
#include <algorithm>

using namespace astro;
using namespace astro::config;
using namespace astro::discover;

namespace astro {
namespace app {
namespace instrument {

/**
 * \brief Table of options for the astroinstrument
 */
static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ NULL,		0,			NULL,		0   } 
};

/**
 * \brief Usage function for the 
 */
static void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] help" << std::endl;
	std::cout << prg << " [ options ] list" << std::endl;
	std::cout << prg << " [ options ] <INSTR> <command>..." << std::endl;
	std::cout << std::endl;
	std::cout << "The following commands are available:" << std::endl;
	std::cout << std::endl;
	std::cout << "list" << std::endl;
	std::cout << "   list all the components defined for an instrument";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "add <type> <service> <deviceurl>" << std::endl;
	std::cout << "    Add a component to an instrument, this also creates the instrument." << std::endl;
	std::cout << "    The following component types are available: Camera, CCD, GuiderCCD, Cooler," << std::endl;
	std::cout << "    GuidePort, Focuser, AdaptiveOptics, FilterWheel" << std::endl;
	std::cout << std::endl;
	std::cout << "remove <type> <index>" << std::endl;
	std::cout << "    remove a component" << std::endl;
	std::cout << std::endl;
	std::cout << "property <name> <value>" << std::endl;
	std::cout << "    add property value" << std::endl;
	std::cout << std::endl;
	std::cout << "remove <name> <value>" << std::endl;
	std::cout << "    remove a property" << std::endl;
	std::cout << std::endl;
	std::cout << "destroy" << std::endl;
	std::cout << "    destroy an instrument" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -c,--config=<cfg>   use configuraton file <cfg> instead "
			"of the default";
	std::cout << std::endl;
	std::cout << " -d,--debug          increase debug level" << std::endl;
	std::cout << " -h,--help           show help message and exit";
	std::cout << std::endl;
}

/**
 * \brief display a help message
 */
static int	cmd_help() {
	usage("astroinstrument");
	return EXIT_SUCCESS;
}

/**
 * \brief List all known instruments
 */
static int	cmd_list(InstrumentBackend& instrumentbackend) {
	InstrumentList	list = instrumentbackend.names();
	for_each(list.begin(), list.end(),
		[](const std::string& name) {
			std::cout << name << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief display the components of an instrument
 */
static int	cmd_list_instrument(InstrumentBackend& instrumentbackend,
		const std::string& instrumentname) {
	if (!instrumentbackend.has(instrumentname)) {
		std::cerr << "instrument " << instrumentname;
		std::cerr << " does not exist" << std::endl;
		return EXIT_FAILURE;
	}
	discover::InstrumentPtr	instrument
		= instrumentbackend.get(instrumentname);
	// list components
	discover::Instrument::ComponentList	components = instrument->list();
	for_each(components.begin(), components.end(),
		[](const discover::InstrumentComponent& component) {
			std::cout << component.name();
			std::cout << " ";
			std::cout << InstrumentComponentKey::type2string(
					component.type());
			std::cout << "[" << component.index() << "] ";
			std::cout << component.servicename();
			std::cout << " ";
			std::cout << component.deviceurl() << std::endl;
		}
	);

	// list properties
	discover::InstrumentPropertyList	proplist
		= instrument->getProperties();
	for_each(proplist.begin(), proplist.end(),
		[](const InstrumentProperty& property) {
			std::cout << property.instrument();
			std::cout << ".";
			std::cout << property.property();
			std::cout << " = ";
			std::cout << property.value();
			if (property.description().size() > 0) {
				std::cout << " // ";
				std::cout << property.description();
			}
			std::cout << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief add component command implementation
 */
static int	cmd_add(InstrumentBackend& instrumentbackend,
		const std::string& instrumentname,
		const std::vector<std::string>& args) {
	if (args.size() < 3) {
		std::cerr << "not enough arguments for add command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	discover::InstrumentComponentKey::Type	type
		= discover::InstrumentComponentKey::string2type(args[0]);
	discover::InstrumentComponent	component(instrumentname, type,
		args[1], args[2]);
	discover::InstrumentPtr	instrument
		= instrumentbackend.get(instrumentname);
	instrument->add(component);
	return EXIT_SUCCESS;
}

/**
 * \brief remove component command implementation
 */
static int	cmd_remove(InstrumentBackend& instrumentbackend,
		const std::string& instrumentname,
		const std::vector<std::string>& args) {
	if (args.size() < 2) {
		std::cerr << "not enough arguments for remove command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	discover::InstrumentComponentKey::Type	type
		= discover::InstrumentComponentKey::string2type(args[0]);
	int	index = std::stoi(args[1]);
	discover::InstrumentPtr	instrument
		= instrumentbackend.get(instrumentname);
	instrument->remove(type, index);
	return EXIT_SUCCESS;
}

/**
 * \brief add a property 
 */
static int	cmd_property(InstrumentBackend& instrumentbackend,
		const std::string& instrumentname,
		const std::vector<std::string>& args) {
	if (args.size() < 1) {
		std::cerr << "not enough arguments for remove command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	discover::InstrumentPtr	instrument
		= instrumentbackend.get(instrumentname);
	std::string	propertyname = args[0];
	if (args.size() == 1) {
		discover::InstrumentProperty	property
			= instrument->getProperty(propertyname);
		std::cout << property.instrument();
		std::cout << ".";
		std::cout << property.property();
		std::cout << " = ";
		std::cout << property.value();
		if (property.description().size() > 0) {
			std::cout << " // ";
			std::cout << property.description();
		}
		std::cout << std::endl;
		return EXIT_SUCCESS;
	}
	std::string	value;
	if (args.size() > 1) {
		value = args[1];
	}
	std::string	description;
	if (args.size() > 2) {
		description = args[2];
	}
	if (instrument->hasProperty(propertyname)) {
		discover::InstrumentProperty	property
			= instrument->getProperty(propertyname);
		property.value(value);
		property.description(description);
		instrument->updateProperty(property);
	} else {
		discover::InstrumentProperty	property;
		property.instrument(instrumentname);
		property.property(propertyname);
		property.value(value);
		property.description(description);
		instrument->addProperty(property);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Remove a property
 */
static int	cmd_remove_property(InstrumentBackend& instrumentbackend,
		const std::string& instrumentname,
		const std::string& propertyname) {
	if (!instrumentbackend.has(instrumentname)) {
		std::cerr << "instrument " << instrumentname;
		std::cerr << " does not exist" << std::endl;
		return EXIT_FAILURE;
	}
	discover::InstrumentPtr	instrument
		= instrumentbackend.get(instrumentname);
	if (!instrument->hasProperty(propertyname)) {
		std::cerr << "property " << propertyname;
		std::cerr << " does not exist" << std::endl;
		return EXIT_FAILURE;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "removing property %s.%s",
		instrumentname.c_str(), propertyname.c_str());
	instrument->removeProperty(propertyname);
	return EXIT_SUCCESS;
}

/**
 * \brief Destroy an instrument
 */
static int	cmd_destroy(InstrumentBackend& instrumentbackend,
		const std::string& instrumentname) {
	if (!instrumentbackend.has(instrumentname)) {
		std::cerr << "instrument " << instrumentname;
		std::cerr << " does not exist" << std::endl;
		return EXIT_FAILURE;
	}
	instrumentbackend.remove(instrumentname);
	return EXIT_SUCCESS;
}

/**
 * \brief Interpret the various subcommands that astroinstrument implements
 */
static int	commands(const std::vector<std::string>& arguments) {
	std::vector<std::string>	args = arguments;
	if (args.size() < 1) {
		throw std::runtime_error("not enought arguments");
	}
	std::string	command = arguments[0];
	if (command == "help") {
		return cmd_help();
	}

	ConfigurationPtr	configuration = Configuration::get();
	InstrumentBackend	instrumentbackend(configuration->database());

	// generic list command
	if (command == "list") {
		return cmd_list(instrumentbackend);
	}

	// all other commands need an instrument name
	std::string	instrumentname = command;
	args.erase(args.begin());
	command = args[0];
	args.erase(args.begin());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command now %s", command.c_str());

	// now call the individual commands
	if (command == "list") {
		return cmd_list_instrument(instrumentbackend, instrumentname);
	}
	if (command == "add") {
		return cmd_add(instrumentbackend, instrumentname, args);
	}
	if (command == "property") {
		return cmd_property(instrumentbackend, instrumentname, args);
	}
	if (command == "remove") {
		switch (args.size()) {
		case 1:
			return cmd_remove_property(instrumentbackend,
				instrumentname, args[0]);
		case 2:
			return cmd_remove(instrumentbackend,
				instrumentname, args);
		default:
			std::cerr << "wroong number of arguments" << std::endl;
			return EXIT_FAILURE;
		}
	}
	if (command == "destroy") {
		return cmd_destroy(instrumentbackend, instrumentname);
	}

	throw std::runtime_error("unknown command");
}

/**
 * \brief main method of the astroinstrument command
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			Configuration::set_default(std::string(optarg));
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 1:
			switch (longindex) {
			}
			break;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// remaining arguments are 
	std::vector<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(argv[optind++]);
	}

	// decide which command is requested
	return commands(arguments);
}

} // namespace instrument
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::instrument::main>(argc, argv);
}
