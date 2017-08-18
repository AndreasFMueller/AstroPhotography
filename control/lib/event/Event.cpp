/*
 * Event.cpp -- Event class implementation
 * 
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroEvent.h>
#include <AstroConfig.h>
#include <AstroDiscovery.h>
#include <AstroDebug.h>
#include <includes.h>
#include <sstream>

using namespace astro::config;
using namespace astro::discover;
using namespace astro::callback;

namespace astro {
namespace events {

std::string	level2string(eventlevel_t level) {
	switch (level) {
	case DEBUG:	return std::string("DEBUG");
	case INFO:	return std::string("INFO");
	case NOTICE:	return std::string("NOTICE");
	case WARNING:	return std::string("WARNING");
	case ERR:	return std::string("ERR");
	case CRIT:	return std::string("CRIT");
	case ALERT:	return std::string("ALERT");
	case EMERG:	return std::string("EMERG");
	}
	std::string	msg = stringprintf("unknown level: %d", level);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

std::string	Event::subsystem2string(Event::Subsystem s) const {
	switch (s) {
	case DEBUG:		return std::string("debug");
	case DEVICE:		return std::string("device");
	case FOCUS:		return std::string("focus");
	case GUIDE:		return std::string("guide");
	case IMAGE:		return std::string("image");
	case INSTRUMENT:	return std::string("instrument");
	case MODULE:		return std::string("module");
	case REPOSITORY:	return std::string("repository");
	case SERVER:		return std::string("server");
	case TASK:		return std::string("task");
	case UTILITIES:		return std::string("utilities");
	}
	std::string	msg = stringprintf("unknown subsystem %d", s);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

Event::Subsystem	Event::string2subsystem(const std::string& s) const {
	if (s == std::string("debug"))		{ return DEBUG; 	}
	if (s == std::string("device"))		{ return DEVICE; 	}
	if (s == std::string("focus"))		{ return FOCUS; 	}
	if (s == std::string("guide"))		{ return GUIDE; 	}
	if (s == std::string("image"))		{ return IMAGE; 	}
	if (s == std::string("instrument"))	{ return INSTRUMENT; 	}
	if (s == std::string("module"))		{ return MODULE; 	}
	if (s == std::string("repository"))	{ return REPOSITORY; 	}
	if (s == std::string("server"))		{ return SERVER; 	}
	if (s == std::string("task"))		{ return TASK; 		}
	if (s == std::string("utilities"))	{ return UTILITIES; 	}
	std::string	msg = stringprintf("unknown subsystem '%s'", s.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

std::string	Event::toString() const {
	std::ostringstream	out;
	out << "level=" << level2string(level) << ", ";
	out << "pid=" << pid << ", ";
	out << "service=" << service << ", ";
	out << "subsystem=" << subsystem << ", ";
	out << "classname=" << classname << ", ";
	out << "file:line=" << file << ":" << line << ", ";
	out << "massage=" << message;
	return out.str();
}

} // namespace events
} // namespace astro
