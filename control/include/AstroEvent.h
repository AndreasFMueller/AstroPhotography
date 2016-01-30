/*
 * AstroEvent.h -- event recording subsystem
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>
#include <AstroUtils.h>

namespace astro {
namespace event {

/**
 * \brief Event class, encapsulates complete event info
 */
class Event {
public:
	int		pid;
	std::string	service;
	double		eventtime;
	std::string	subsystem;
	std::string	message;
	std::string	object;
	std::string	file;
	std::string	line;
	typedef enum { DEBUG, FOCUS, GUIDE, IMAGE, MODULE, TASK } Subsystem;
};

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

class EventHandler {
	bool	_active;
	astro::persistence::Database	database;
public:
static bool	active();
static void	active(bool a);
private:
	EventHandler(const EventHandler& other);
	EventHandler&	operator=(const EventHandler& other);
public:
	EventHandler() { }
static EventHandler&	get();
static void	consume(const std::string& file, int line,
			const std::string& object,
			const Event::Subsystem subsystem,
			const std::string& message);
private:
	void	process(const std::string& file, int line,
			const std::string& object,
			const Event::Subsystem subsystem,
			const std::string& message);
};

} // namespace astro
} // namespace event

#define	EVENT_LOG	__FILE__, __LINE__, demangle(typeid(*this).name())

extern void	event(const char *file, int line, const std::string& object,
			const astro::event::Event::Subsystem subsystem,
			const std::string& message);


