/*
 * EventHandler.cpp -- event handler class implementation
 * 
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroEvent.h>
#include <AstroConfig.h>
#include <AstroDiscovery.h>
#include <includes.h>

using namespace astro::config;
using namespace astro::discover;
using namespace astro::callback;

namespace astro {
namespace events {

static EventHandler	handler;

bool	EventHandler::active() {
	return handler._active;
}

void	EventHandler::active(bool a) {
	handler._active = a;
}

void	EventHandler::callback(CallbackPtr c) {
	handler._callback = c;
}

EventHandler&	EventHandler::get() {
	return handler;
}

void	EventHandler::consume(const std::string& file, int line,
		const std::string& classname, const Event::Subsystem subsystem,
		const std::string& message) {
	return handler.process(file, line, classname, subsystem, message);
}

/**
 * \brief Main event processing method
 */
void	EventHandler::process(const std::string& file, int line,
		const std::string& classname, const Event::Subsystem subsystem,
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
	gettimeofday(&record.eventtime, NULL);
	switch (subsystem) {
	case Event::DEBUG:
		record.subsystem = "debug";
		break;
	case Event::DEVICE:
		record.subsystem = "device";
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
	case Event::INSTRUMENT:
		record.subsystem = "instrument";
		break;
	case Event::MODULE:
		record.subsystem = "module";
		break;
	case Event::REPOSITORY:
		record.subsystem = "repository";
		break;
	case Event::SERVER:
		record.subsystem = "server";
		break;
	case Event::TASK:
		record.subsystem = "task";
		break;
	case Event::UTILITIES:
		record.subsystem = "utilities";
		break;
	}
	record.message = message;
	record.classname = classname;
	record.file = file;
	record.line = line;

	// save the record in the database table
	EventTable	table(database);
	table.add(record);

	// if a callback is installed, send the event to the callback
	if (!_callback) {
		return;
	}
	EventCallbackData	*ecd = new EventCallbackData(record);
	CallbackDataPtr	cd = CallbackDataPtr(ecd);
	_callback->operator()(cd);
}

} // namespace events

void	event(const char *file, int line, const std::string& classname,
		const events::Event::Subsystem subsystem,
		const std::string& message) {
	try {
		events::EventHandler::consume(file, line, classname,
			subsystem, message);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write event: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write event");
	}
}

} // namespace astro
