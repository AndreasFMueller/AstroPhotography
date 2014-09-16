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
#include <InstrumentTables.h>
#include <algorithm>
#include <stacktrace.h>

using namespace astro::config;
using namespace astro::persistence;

namespace astro {

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
void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << " [ options ] list" << std::endl;
	std::cout << prg << " [ options ] { add | show | remove } <name>";
	std::cout << std::endl;
	std::cout << prg << " [ options ] <name> add <cname> [ attr=value ]";
	std::cout << std::endl;
	std::cout << prg << " [ options ] <name> { show | remove } <type>";
	std::cout << std::endl;
	std::cout << prg << " [ options ] <name> update <type> "
		"[ attr=value ]";
	std::cout << std::endl;
	std::cout << prg << " [ options ] help" << std::endl;
	std::cout << std::endl;
	std::cout << "the following attributes are known:" << std::endl;
	std::cout << "  unit=<u>      unit number of a device" << std::endl;
	std::cout << "  device=<d>    device name of a direct component";
	std::cout << std::endl;
	std::cout << "  name=<n>      name of a mapped component" << std::endl;
	std::cout << "  kind=<k>      component kind (direct, mapped, derived)";
	std::cout << std::endl;
	std::cout << "  from=<f>      the device type from which this component"
		" is derived" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
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
int	cmd_help() {
	usage("astroinstrument");
	return EXIT_SUCCESS;
}

/**
 * \brief List all known instruments
 */
int	cmd_list() {
	std::list<InstrumentPtr>	instruments
		= Configuration::get()->listinstruments();
	std::for_each(instruments.begin(), instruments.end(),
		[](InstrumentPtr instrument) {
			std::cout << instrument->toString() << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief add an instrument
 *
 * Adding an instrument needs only the name of the instrument
 */
int	cmd_add(const std::string& instrumentname,
		const std::vector<std::string>& /* arguments */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding instrument '%s'",
		instrumentname.c_str());
	ConfigurationPtr	config = Configuration::get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new instrument");
	InstrumentPtr	instrument(new Instrument(config->database(),
				instrumentname));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument created");
	config->addInstrument(instrument);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument added");
	return EXIT_SUCCESS;
}

/**
 * \brief show an instrument
 */
int	cmd_show(const std::string& instrumentname,
		const std::vector<std::string>& /* arguments */) {
	InstrumentPtr	instrument
		= Configuration::get()->instrument(instrumentname);
	std::cout << instrument->name();
	std::list<DeviceName::device_type>	types
		= instrument->component_types();
	std::cout << " has " << types.size() << " components" << std::endl;
	for (auto ptr = types.begin(); ptr != types.end(); ptr++) {
		InstrumentComponentPtr	component = instrument->component(*ptr);
		std::cout << component->toString() << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Remove an instrument
 */
int	cmd_remove(const std::string& instrumentname,
		const std::vector<std::string>& /* arguments */) {
	Configuration::get()->removeInstrument(instrumentname);
	return EXIT_SUCCESS;
}

/**
 * \brief add a component to an instrument
 *
 * Adding an instrument uses the command format
 *
 *     INSTR add cname attr=value
 *
 * INSTR is the name of the instrument, cname is the device type of the
 * component to be added. Some of the valid device types are camera,
 * ccd, adaptiveoptics, cooler, filterwheel, ... 
 * The following attribute value pairs are unit=<u>, kind=<t> (required),
 * device=<d>, name=<n>, from=<f>
 */
int	cmd_component_add(const std::string& instrumentname,
		const std::string& componenttype,
		const std::vector<std::string>& arguments) {
	// get the instrument to be changed
	ConfigurationPtr	config = Configuration::get();
	InstrumentPtr	instrument = config->instrument(instrumentname);

	// interpret the component type
	debug(LOG_DEBUG, DEBUG_LOG, 0, "component type: %s",
		componenttype.c_str());
	DeviceName::device_type	type
		= InstrumentComponentTableAdapter::type(componenttype);

	// build the component
	InstrumentComponentPtr	component(NULL);

	// parse the arguments
	AttributeValuePairs	av(arguments, 3);

	// check for the type
	InstrumentComponent::component_t 	ctype
		= InstrumentComponent::direct;
	if (av.has("kind")) {
		std::string	t = av("kind");
		ctype = InstrumentComponentTableAdapter::component_type(t);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "the component type is %s", 
		InstrumentComponentTableAdapter::component_type(ctype).c_str());;

	// get the unit number, as all types of components have this attribute
	int	unit = 0;
	if (av.has("unit")) {
		unit = std::stoi(av("unit"));
	}

	// now use different code for the various types of component classes
	switch (ctype) {
	case InstrumentComponent::direct:
		component = InstrumentComponentPtr(
			new InstrumentComponentDirect(type,
				DeviceName(av("device")), unit));
		break;
	case InstrumentComponent::mapped:
		// construct a mapped component
		component = InstrumentComponentPtr(
			new InstrumentComponentMapped(type,
				config->database(), av("name")));
		break;
	case InstrumentComponent::derived:
		// construct a derived compont, where from attribute contains
		// the type of the component this component is derived from
		component = InstrumentComponentPtr(
			new InstrumentComponentDerived(type, instrument,
				InstrumentComponentTableAdapter::type(av("from")),
				unit));
		break;
	}

	// add the component to the instrument
	instrument->add(component);

	// persist in the database
	config->database()->begin("addcomponent");
	try {
		config->removeInstrument(instrumentname);
		config->addInstrument(instrument);
		config->database()->commit("addcomponent");
	} catch (...) {
		config->database()->rollback("addcomponent");
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Update a component of an instrument
 */
int	cmd_component_update(const std::string& instrumentname,
		const std::string& componenttype,
		const std::vector<std::string>& arguments) {
	// get the component to change
	ConfigurationPtr	config = Configuration::get();
	InstrumentPtr	instrument = config->instrument(instrumentname);
	InstrumentComponentPtr	component = instrument->component(
		InstrumentComponentTableAdapter::type(componenttype));

	// get the attribute value types from the remaining arguments
	AttributeValuePairs	av(arguments, 3);

	// check for the type
	if (av.has("kind")) {
		InstrumentComponent::component_t	ctype
			= InstrumentComponentTableAdapter::component_type(
				av("kind"));
		if (ctype != component->component_type()) {
			throw std::runtime_error("cannot change type, delete "
				"and add component of new type");
		}
	}

	// XXX modify the component
	if (av.has("unit")) {
		component->unit(std::stoi(av("unit")));
	}
	switch (component->component_type()) {
	case InstrumentComponent::direct:
		if (av.has("device")) {
			component->name(av("device"));
		}
		break;
	case InstrumentComponent::mapped:
		if (av.has("name")) {
			component->name(av("name"));
		}
		break;
	case InstrumentComponent::derived:
		if (av.has("from")) {
			component->name(av("from"));
		}
		break;
	}

	// persist in the database
	config->database()->begin("updatecomponent");
	try {
		config->removeInstrument(instrumentname);
		config->addInstrument(instrument);
		config->database()->commit("updatecomponent");
	} catch (...) {
		config->database()->rollback("updatecomponent");
		throw;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Show the details of a component
 */
int	cmd_component_show(const std::string& instrumentname,
		const std::string& componenttype,
		const std::vector<std::string>& /* arguments */) {
	InstrumentPtr	instrument
		= Configuration::get()->instrument(instrumentname);
	InstrumentComponentPtr	component = instrument->component(
		InstrumentComponentTableAdapter::type(componenttype));
	std::cout << componenttype << " component of instrument ";
	std::cout << instrumentname  << ":" << std::endl;
	std::cout << component->toString() << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Remove a component from from an instrument
 */
int	cmd_component_remove(const std::string& instrumentname,
		const std::string& componenttype,
		const std::vector<std::string>& /* arguments */) {
	ConfigurationPtr	config = Configuration::get();
	InstrumentPtr	instrument
		= config->instrument(instrumentname);
	instrument->remove(
		InstrumentComponentTableAdapter::type(componenttype));
	config->database()->begin("removecomponent");
	try {
		config->removeInstrument(instrumentname);
		config->addInstrument(instrument);
		config->database()->commit("removecomponent");
	} catch (...) {
		config->database()->rollback("removecomponent");
		throw;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Interpret the various subcommands that astroinstrument implements
 */
int	commands(const std::vector<std::string>& arguments) {
	if (arguments.size() < 1) {
		throw std::runtime_error("not enought arguments");
	}
	// generic list command
	if (arguments[0] == "list") {
		return cmd_list();
	}
	if (arguments[0] == "help") {
		return cmd_help();
	}

	// instrument specific commands
	if (arguments.size() < 2) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	instrumentname = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrumentname = %s",
		instrumentname.c_str());
	if (arguments[0] == "add") {
		return cmd_add(instrumentname, arguments);
	}
	if (arguments[0] == "show") {
		return cmd_show(instrumentname, arguments);
	}
	if (arguments[0] == "remove") {
		return cmd_remove(instrumentname, arguments);
	}
	// component commands start here
	instrumentname = arguments[0];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrumentname = %s",
		instrumentname.c_str());
	if (arguments.size() < 3) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	componenttype = arguments[2];
	std::string	cmd = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command = %s", cmd.c_str());
	if (cmd == "add") {
		return cmd_component_add(instrumentname, componenttype,
			arguments);
	}
	if (cmd == "update") {
		return cmd_component_update(instrumentname, componenttype,
			arguments);
	}
	if (cmd == "remove") {
		return cmd_component_remove(instrumentname, componenttype,
			arguments);
	}
	if (cmd == "show") {
		return cmd_component_show(instrumentname, componenttype,
			arguments);
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

} // namespace astro

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return astro::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by ";
		std::cerr << astro::demangle(typeid(x).name()) << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception: " << std::endl;
	}
	return EXIT_FAILURE;
}
