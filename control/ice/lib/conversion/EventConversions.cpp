/*
 * EventConversions.cpp -- conversions between ice and astro events
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

EventLevel	convert(astro::events::eventlevel_t l) {
	switch (l) {
	case astro::events::DEBUG:	return EventLevelDEBUG;
	case astro::events::INFO:	return EventLevelINFO;
	case astro::events::NOTICE:	return EventLevelNOTICE;
	case astro::events::WARNING:	return EventLevelWARNING;
	case astro::events::ERR:	return EventLevelERR;
	case astro::events::CRIT:	return EventLevelCRIT;
	case astro::events::ALERT:	return EventLevelALERT;
	case astro::events::EMERG:	return EventLevelEMERG;
	}
	throw std::runtime_error("unknown level");
}

astro::events::eventlevel_t	convert(EventLevel l) {
	switch (l) {
	case EventLevelDEBUG:	return astro::events::DEBUG;
	case EventLevelINFO:	return astro::events::INFO;
	case EventLevelNOTICE:	return astro::events::NOTICE;
	case EventLevelWARNING:	return astro::events::WARNING;
	case EventLevelERR:	return astro::events::ERR;
	case EventLevelCRIT:	return astro::events::CRIT;
	case EventLevelALERT:	return astro::events::ALERT;
	case EventLevelEMERG:	return astro::events::EMERG;
	}
	throw std::runtime_error("unknown level");
}

Event	convert(const astro::events::Event& e) {
	Event	result;
	result.id = -1;
	result.level = convert(e.level);
	result.pid = e.pid;
	result.service = e.service;
	result.timeago = converttimeval(e.eventtime);
	result.subsystem = e.subsystem;
	result.message = e.message;
	result.classname = e.classname;
	result.file = e.file;
	result.line = e.line;
	return result;
}

astro::events::Event	convert(const Event& e) {
	astro::events::Event	result;
	result.level = convert(e.level);
	result.pid = e.pid;
	result.service = e.service;
	result.eventtime = converttimeval(e.timeago);
	result.subsystem = e.subsystem;
	result.message = e.message;
	result.classname = e.classname;
	result.file = e.file;
	result.line = e.line;
	return result;
}

Event	convert(const astro::events::EventRecord& e) {
	Event	result;
	result.id = e.id();
	result.level = convert(e.level);
	result.pid = e.pid;
	result.service = e.service;
	result.timeago = converttimeval(e.eventtime);
	result.subsystem = e.subsystem;
	result.message = e.message;
	result.classname = e.classname;
	result.file = e.file;
	result.line = e.line;
	return result;
}

astro::events::EventRecord	convertRecord(const Event& e) {
	astro::events::EventRecord	result(e.id);
	result.level = convert(e.level);
	result.pid = e.pid;
	result.service = e.service;
	result.eventtime = converttimeval(e.timeago);
	result.subsystem = e.subsystem;
	result.message = e.message;
	result.classname = e.classname;
	result.file = e.file;
	result.line = e.line;
	return result;
}

} // namespace snowstar
