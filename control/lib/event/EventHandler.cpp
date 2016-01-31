/*
 * EventHandler.cpp -- event handler class implementation
 * 
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroEvent.h>
#include <AstroConfig.h>
#include <AstroDiscovery.h>

using namespace astro::config;
using namespace astro::discover;

namespace astro {
namespace events {

static EventHandler	handler;

bool	EventHandler::active() {
	return handler._active;
}

void	EventHandler::active(bool a) {
	handler._active = a;
}

EventHandler&	EventHandler::get() {
	return handler;
}

void	EventHandler::consume(const std::string& file, int line,
		const std::string& object, const Event::Subsystem subsystem,
		const std::string& message) {
	return handler.process(file, line, object, subsystem, message);
}

void	EventHandler::process(const std::string& file, int line,
		const std::string& object, const Event::Subsystem subsystem,
		const std::string& message) {
	if (!_active) {
		return;
	}
	if (!database) {
		database = Configuration::get()->database();
	}
	if (!database) {
		// still no database, give up
		return;
	}
	
	EventRecord	record(-1);
	record.pid = getpid();
	record.service = discover::ServiceLocation::get().servicename();
	record.eventtime = Timer::gettime();
	switch (subsystem) {
	case Event::DEBUG:
		record.subsystem = "debug";
		break;
	case Event::FOCUS:
		record.subsystem = "focus";
		break;
	case Event::GUIDE:
		record.subsystem = "guide";
		break;
	case Event::IMAGE:
		record.subsystem = "image";
		break;
	case Event::MODULE:
		record.subsystem = "module";
		break;
	case Event::TASK:
		record.subsystem = "task";
		break;
	}
	record.message = message;
	record.object = object;
	record.file = file;
	record.line = line;

	EventTable	table(database);
	table.add(record);
}

} // namespace events

void	event(const char *file, int line, const std::string& object,
		const events::Event::Subsystem subsystem,
		const std::string& message) {
	return events::EventHandler::consume(file, line, object,
		subsystem, message);
}

} // namespace astro
