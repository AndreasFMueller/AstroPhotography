/*
 * EventHandlerI.cpp -- event handler implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <EventHandlerI.h>
#include <IceConversions.h>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroFormat.h>

namespace snowstar {

/**
 * \brief calback adapter for Event monitor
 */
template<>
void    callback_adapter<EventMonitorPrx>(EventMonitorPrx p,
	const astro::callback::CallbackDataPtr data) {
	// convert the callback data to the original data
	astro::events::EventCallbackData	*cbd 
		= dynamic_cast<astro::events::EventCallbackData *>(&*data);
	if (NULL == cbd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not event callback data");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found callback data of type %s",
		astro::demangle_cstr(cbd));

	// now send the payload to the registerd clients
	Event	event = convert(cbd->data());
	p->update(event);
}

/**
 * \brief Adapter class for calibration callback
 */
class EventHandlerICallback : public astro::callback::Callback {
	EventHandlerI&	_eventhandler;
public:
	EventHandlerICallback(EventHandlerI& eventhandler)
		: _eventhandler(eventhandler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "event callback %p created",
			this);
	}
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "event callback called");
		_eventhandler.update(data);
		return data;
	}
};

/**
 * \brief Create a new EventHandlerI
 */
EventHandlerI::EventHandlerI() {
	EventHandlerICallback	*callback = new EventHandlerICallback(*this);
	_callback = astro::callback::CallbackPtr(callback);
	astro::events::EventHandler::callback(_callback);
}

/**
 * \brief Destroy the event handler
 */
EventHandlerI::~EventHandlerI() {
	astro::events::EventHandler::callback(astro::callback::CallbackPtr());
}

/**
 * \brief Get an event identified by its id
 */
Event	EventHandlerI::eventId(int id, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "event id %d requested", id);
	astro::config::ConfigurationPtr	configuration
                = astro::config::Configuration::get();
	astro::persistence::Database	database = configuration->database();
	astro::events::EventTable	table(database);
	try {
		astro::events::EventRecord	record = table.byid(id);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"got event record '%s', level %d",
			record.message.c_str(), record.level);
		return convert(record);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "row not found: %s", x.what());
		NotFound	notfound;
		notfound.cause = x.what();
		throw notfound;
	}
}

/**
 * \brief Retrieve all events between to timestamps
 */
eventlist	EventHandlerI::eventsBetween(double fromago, double toago,
					const Ice::Current& current) {
	CallStatistics::count(current);
	std::string	condition
		= astro::stringprintf("eventtime between %f and %f", 
			astro::Timer::gettime() - fromago,
			astro::Timer::gettime() - toago);
	return eventsCondition(condition, current);
}

/**
 * \brief Retrieve all events matching a condition
 */
eventlist	EventHandlerI::eventsCondition(const std::string& condition,
					const Ice::Current& current) {
	CallStatistics::count(current);
	// get the database
	astro::config::ConfigurationPtr	configuration
                = astro::config::Configuration::get();
	astro::persistence::Database	database = configuration->database();
	astro::events::EventTable	table(database);

	// build the condition
	std::string	fullcondition
		= condition + astro::stringprintf(" and pid = %d", getpid());
	std::list<astro::events::EventRecord>	events
		= table.select(fullcondition);

	// convert all the events 
	eventlist	result;
	std::list<astro::events::EventRecord>::const_iterator	i;
	for (i = events.begin(); i != events.end(); i++) {
		result.push_back(convert(*i));
	}
	return result;
}

/**
 * \brief Register an event monitor
 */
void	EventHandlerI::registerMonitor(const Ice::Identity& eventmonitor,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback registration");
	try {
		eventcallbacks.registerCallback(eventmonitor, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register event monitor callback: %s %s",
			astro::demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register event callback for unknown reason");
	}
}

/**
 * \brief Unregister an event monitor
 */	
void	EventHandlerI::unregisterMonitor(const Ice::Identity& eventmonitor,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback unregistration");
	eventcallbacks.unregisterCallback(eventmonitor, current);
}

/**
 * \brief Propagate a callback update to all the callbacks
 */
void	EventHandlerI::update(astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got event callback");
	eventcallbacks(data);
}

} // namespace snowstar
