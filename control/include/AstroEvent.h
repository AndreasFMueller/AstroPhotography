/*
 * AstroEvent.h -- event recording subsystem
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroEvent_h
#define _AstroEvent_h

#include <AstroPersistence.h>
#include <AstroUtils.h>
#include <AstroCallback.h>

namespace astro {
namespace events {

typedef enum eventlevel_e {
	DEBUG=0, INFO, NOTICE, WARNING, ERR, CRIT, ALERT, EMERG
} eventlevel_t;

std::string	level2string(eventlevel_t level);

/**
 * \brief Event class, encapsulates complete event info
 */
class Event {
public:
	eventlevel_t	level;
	int		pid;
	std::string	service;
	struct timeval	eventtime;
	std::string	subsystem;
	std::string	message;
	std::string	classname;
	std::string	file;
	int	line;
	typedef enum {
		DEBUG,
		DEVICE,
		FOCUS,
		GUIDE,
		IMAGE,
		INSTRUMENT,
		MODULE,
		REPOSITORY,
		SERVER,
		TASK,
		UTILITIES
	} Subsystem;
	std::string	subsystem2string(Subsystem) const;
	Subsystem	string2subsystem(const std::string&) const;
	std::string	toString() const;
};

/**
 * \brief Interface to callbacks
 */
typedef callback::CallbackDataEnvelope<Event>	EventCallbackData;

/**
 * \brief Persistence of Events
 */
typedef astro::persistence::Persistent<Event>	EventRecord;

/**
 * \brief Adapter for event able
 */
class EventTableAdapter {
public:
static std::string      tablename();
static std::string      createstatement();
static EventRecord
        row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
        object_to_updatespec(const EventRecord& event);
};

typedef astro::persistence::Table<EventRecord, EventTableAdapter> EventTable;

/**
 * \brief Handler for callbacks
 */
class EventHandler {
	bool	_active;
	persistence::Database	database;
	callback::CallbackPtr	_callback;
public:
static bool	active();
static void	active(bool a);
static void	callback(callback::CallbackPtr);
private:
	EventHandler(const EventHandler& other);
	EventHandler&	operator=(const EventHandler& other);
public:
	EventHandler() { }
static EventHandler&	get();
static void	consume(const std::string& file, int line,
			const std::string& classname,
			eventlevel_t level,
			const Event::Subsystem subsystem,
			const std::string& message);
private:
	void	process(const std::string& file, int line,
			const std::string& classname,
			eventlevel_t level,
			const Event::Subsystem subsystem,
			const std::string& message);
};


} // namespace events

#define	EVENT_CLASS	__FILE__, __LINE__, astro::demangle(typeid(*this).name())
#define	EVENT_GLOBAL	__FILE__, __LINE__, ""

extern void	event(const char *file, int line, const std::string& classname,
			events::eventlevel_t level,
			const events::Event::Subsystem subsystem,
			const std::string& message);

} // namespace astro

#endif /* _AstroEvent_h */
