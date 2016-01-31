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

Event	convert(const astro::events::Event& e) {
	Event	result;
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
