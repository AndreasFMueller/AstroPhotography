
/*
 * filterwheelcommand.h -- filterwheel command implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <filterwheelcommand.h>
#include <Filterwheels.h>
#include <iostream>
#include <unistd.h>

namespace astro {
namespace cli {

std::ostream&	operator<<(std::ostream& out, const Astro::FilterwheelState& state) {
	switch (state) {
	case Astro::FILTERWHEEL_IDLE:
		out << "idle"; break;
	case Astro::FILTERWHEEL_MOVING:
		out << "moving"; break;
	case Astro::FILTERWHEEL_UNKNOWN:
		out << "unknown"; break;
	}
	return out;
}

std::ostream&	operator<<(std::ostream& out, FilterwheelWrapper filterwheel) {
	out << "name:         " << filterwheel->getName() << std::endl;
	int	npositions = filterwheel->nFilters();
	out << "filters:      " << npositions << std::endl;
	out << "filter names: ";
	for (int position = 0; position < npositions; position++) {
		if (position > 0) { out << ", "; }
		out << filterwheel->filterName(position);
	}
	out << std::endl;
	out << "state:        " << filterwheel->getState() << std::endl;
	if (filterwheel->getState() == Astro::FILTERWHEEL_IDLE) {
		out << "current:      "
			<< filterwheel->currentPosition() << std::endl;
	}
	return out;
}

void	filterwheelcommand::info(FilterwheelWrapper& filterwheel,
		const std::vector<std::string>& /* arguments */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel %s info",
		filterwheel->getName());
	std::cout << filterwheel;
}

void	filterwheelcommand::release(const std::string& filterwheelid,
		const std::vector<std::string>& /* arguments */) {
	Filterwheels	filterwheels;
	filterwheels.release(filterwheelid);
}

void	filterwheelcommand::assign(const std::string& filterwheelid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assign %s", filterwheelid.c_str());
	try {
		Filterwheels	filterwheels;
		filterwheels.assign(filterwheelid, arguments);
	} catch (std::exception& x) {
		throw command_error(x.what());
	}
}

void	filterwheelcommand::position(FilterwheelWrapper& filterwheel,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 3) {
		
	}
	int	position = stoi(arguments[2]);
	filterwheel->select(position);
}

void	filterwheelcommand::wait(FilterwheelWrapper& filterwheel,
		const std::vector<std::string>& /* arguments */) {
	while (filterwheel->getState() != Astro::FILTERWHEEL_IDLE) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting");
		usleep(1000000);
	}
}

void	filterwheelcommand::operator()(const std::string& /* commandname */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw command_error("filterwheel command requires 2 arguments");
	}
	std::string	filterwheelid = arguments[0];
	std::string	subcommandname = arguments[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"filterwheel command for FW %s, subcommand %s",
		filterwheelid.c_str(), subcommandname.c_str());
	if (subcommandname == "release") {
		release(filterwheelid, arguments);
		return;
	}
	if (subcommandname == "assign") {
		assign(filterwheelid, arguments);
		return;
	}
	Filterwheels	filterwheels;
	FilterwheelWrapper	filterwheel
		= filterwheels.byname(filterwheelid);
	if (subcommandname == "info") {
		info(filterwheel, arguments);
		return;
	}
	if (subcommandname == "position") {
		position(filterwheel, arguments);
		return;
	}
	if (subcommandname == "wait") {
		wait(filterwheel, arguments);
		return;
	}
	throw command_error("unknown command");
}

std::string	filterwheelcommand::summary() const {
	return std::string("access filterwheels");
}

std::string	filterwheelcommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tfilterwheel <filterwheelid> assign <cameraid>\n"
	"\tfilterwheel <filterwheelid> info\n"
	"\tfilterwheel <filterwheelid> release\n"
	"\tfilterwheel <filterwheelid> position <n>\n"
	"\tfilterwheel <filterwheelid> wait\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The filterwheel command gives access to a filterwheel mounted on a\n"
	"camera.\n"
	);
}

} // namespace cli
} // namespace astro
