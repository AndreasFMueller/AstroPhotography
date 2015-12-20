/*
 * ServiceInstrument.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <InstrumentComponentTable.h>

namespace astro {
namespace discover {

//////////////////////////////////////////////////////////////////////
// Instrument implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Auxiliary function to add components to a list
 */
void	Instrument::add(std::list<InstrumentComponent>& l,
		InstrumentComponent::Type type) {
	int	n = nComponentsOfType(type);
	for (int i = 0; i < n; i++) {
		l.push_back(get(type, i));
	}
}

/**
 * \brief Build a list of all components of an instrument
 */
std::list<InstrumentComponent>	Instrument::list() {
	std::list<InstrumentComponent>	result;
	add(result, InstrumentComponent::CCD);
	add(result, InstrumentComponent::GuiderCCD);
	add(result, InstrumentComponent::Cooler);
	add(result, InstrumentComponent::GuiderPort);
	add(result, InstrumentComponent::Focuser);
	add(result, InstrumentComponent::AdaptiveOptics);
	add(result, InstrumentComponent::FilterWheel);
	return result;
}

/**
 * \brief Build a list of all components of a given type
 */
std::list<InstrumentComponent>	Instrument::list(InstrumentComponent::Type type) {
	std::list<InstrumentComponent>	result;
	add(result, type);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Instrument Backend implementation
//////////////////////////////////////////////////////////////////////
class InstrumentBackendImpl {
	// shared database connection
	static astro::persistence::Database	database;
	static InstrumentComponentTablePtr	table;
	static void	setup() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setup of with default db");
		astro::config::ConfigurationPtr	config
			= astro::config::Configuration::get();
		database = config->database();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "create table");
		table = InstrumentComponentTablePtr(
				new InstrumentComponentTable(database));
	}
	static void	setupdb(Database db) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setup with separate db");
		database = db;
		table = InstrumentComponentTablePtr(
				new InstrumentComponentTable(database));
	}
	static std::once_flag	ready;
public:
	InstrumentBackendImpl() {
		std::call_once(ready, setup);
	}
	InstrumentBackendImpl(persistence::Database database) {
		std::call_once(ready, setupdb, database);
	}
	long	idfromkey(const InstrumentComponentKey& key);
	long	idfromkey(const std::string& name,
			InstrumentComponent::Type type, int index);
	int	nComponentsOfType(const std::string& name,
			InstrumentComponent::Type type);
	bool	has(const std::string& instrumentname);
	int	add(const InstrumentComponent& component);
	void	update(const InstrumentComponent& component);
	void	remove(const std::string& name,
			InstrumentComponent::Type type, int index);
	void	remove(const InstrumentComponentKey& key);
	void	remove(const std::string& name);
	std::list<std::string>	names();
	InstrumentPtr	get(const std::string& name);
	InstrumentComponent	get(const std::string& name, 
			InstrumentComponent::Type type, int index);
};

astro::persistence::Database	InstrumentBackendImpl::database;
InstrumentComponentTablePtr	InstrumentBackendImpl::table;
std::once_flag	InstrumentBackendImpl::ready;

/**
 * \brief Count the number of components of a given type in an instrument
 */
int	InstrumentBackendImpl::nComponentsOfType(const std::string& name,
			InstrumentComponent::Type type) {
	std::string	query(	"select count(*) "
				"from instrumentcomponents "
				"where name = ? and type = ?");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, name);
	statement->bind(1, type);
	Result	res = statement->result();
	return std::stoi(res.front()[0]->stringValue());
}

/**
 * \brief Add a component to an instrument of a given name
 */
int	InstrumentBackendImpl::add(const InstrumentComponent& component) {
	InstrumentComponentRecord	record(component);
	record.name(component.name());
	record.index(nComponentsOfType(component.name(), component.type()));
	long	id = table->add(record);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new entry with id = %ld", id);
	return record.index();;
}

/**
 * \brief Update a component
 */
void	InstrumentBackendImpl::update(const InstrumentComponent& component) {
	long	objectid = idfromkey(component.name(), component.type(),
				component.index());
	InstrumentComponentInfo	info(component);
	table->update(objectid, info);
}

/**
 * \brief Remove a component
 */
void	InstrumentBackendImpl::remove(const std::string& name,
			InstrumentComponent::Type type, int index) {
	int	n = nComponentsOfType(name, type);

	// remove the component
	long	objectid = idfromkey(name, type, index);
	table->remove(objectid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "object %d with index=%d removed",
		objectid, index);

	// renumber all the other components
	std::string	query(	"update instrumentcomponents "
				"set idx = idx - 1 "
				"where name = ? "
				"  and type = ? "
				"  and idx = ? ");
	for (int i = index + 1; i < n; i++) {
		StatementPtr	statement = database->statement(query);
		statement->bind(0, name);
		statement->bind(1, type);
		statement->bind(2, i);
		statement->execute();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "renumber comleted");
}

void	InstrumentBackendImpl::remove(const std::string& name) {
	std::string	query(	"delete from instrumentcomponents "
				"where name = ?");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, name);
	statement->execute();
}

/**
 * \brief Get a list of the names of available instruments
 */
std::list<std::string>	InstrumentBackendImpl::names() {
	std::list<std::string>	result;
	std::string	query(	"select distinct name "
				"from instrumentcomponents "
				"order by 1 asc;");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name query: %s", query.c_str());
	astro::persistence::Result	r = database->query(query);
	astro::persistence::Result::iterator	i;
	for (i = r.begin(); i != r.end(); i++) { 
		std::string	nm = (*i)[0]->stringValue();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found instrument '%s'",
			nm.c_str());
		result.push_back(nm);
	}
	return result;
}

/**
 * \brief find out whether we have an instrument of this name in the database
 */
bool	InstrumentBackendImpl::has(const std::string& name) {
	std::string	query(	"select count(*) "
				"from instrumentcomponents "
				"where name = ?");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, name);
	Result	res = statement->result();
	return (std::stoi(res.front()[0]->stringValue()) > 0) ? true : false;
}

/**
 * \brief Get the ide of an object from a key
 */
long	InstrumentBackendImpl::idfromkey(const InstrumentComponentKey& key) {
	return idfromkey(key.name(), key.type(), key.index());
}

/**
 * \brief Retrieve the object id from name, type and index of a component
 */
long	InstrumentBackendImpl::idfromkey(const std::string& name,
		InstrumentComponent::Type type, int index) {
	std::string	query(	"select id "
				"from instrumentcomponents "
				"where name = ? "
				"  and type = ? "
				"  and idx = ?");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, name);
	statement->bind(1, type);
	statement->bind(2, index);
	Result	res = statement->result();
	long	objectid = res.front()[0]->intValue();
	if (objectid < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "instrument %s: no matching "
			"component type=%d, index=%d",
			name.c_str(), type, index);
		throw std::runtime_error("no matching instrument component");
	}
	return objectid;
}

/**
 * \brief Get the component
 */
InstrumentComponent	InstrumentBackendImpl::get(const std::string& name, 
				InstrumentComponent::Type type, int index) {
	long	i = idfromkey(name, type, index);
	InstrumentComponentInfo	info = table->byid(i);
	InstrumentComponent	component(info, info.servicename(),
					info.deviceurl());
	return component;
}

//////////////////////////////////////////////////////////////////////
// Instrument implementation
//////////////////////////////////////////////////////////////////////
class InstrumentImpl : public Instrument {
	InstrumentBackendImpl	backend;
public:
	InstrumentImpl(const std::string& name) : Instrument(name) { }
public:
	virtual int	nComponentsOfType(InstrumentComponent::Type type) {
		return backend.nComponentsOfType(name(), type);
	}
	virtual int	add(const InstrumentComponent& component) {
		return backend.add(component);
	}
	virtual void	update(const InstrumentComponent& component) {
		backend.update(component);
	}
	virtual void	remove(InstrumentComponent::Type type, int index) {
		backend.remove(name(), type, index);
	}
	virtual InstrumentComponent	get(InstrumentComponent::Type type,
		int index) {
		return backend.get(name(), type, index);
	}
};

InstrumentPtr	InstrumentBackendImpl::get(const std::string& name) {
	return InstrumentPtr(new InstrumentImpl(name));
}

//////////////////////////////////////////////////////////////////////
// InstrumentList implemenation
//////////////////////////////////////////////////////////////////////
InstrumentList::InstrumentList(const std::list<std::string>& list) {
	std::list<std::string>::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		push_back(*i);
	}
}

//////////////////////////////////////////////////////////////////////
// Instrument Backend
//////////////////////////////////////////////////////////////////////

InstrumentBackend::InstrumentBackend() {
}

InstrumentBackend::InstrumentBackend(persistence::Database database) {
	InstrumentBackendImpl	backend(database);
}

InstrumentList	InstrumentBackend::names() {
	InstrumentBackendImpl	backend;
	return backend.names();
}

bool	InstrumentBackend::has(const std::string& name) {
	InstrumentBackendImpl	backend;
	return backend.has(name);
}

InstrumentPtr	InstrumentBackend::get(const std::string& name) {
	InstrumentBackendImpl	backend;
	return backend.get(name);
}

void	InstrumentBackend::remove(const std::string& name) {
	InstrumentBackendImpl	backend;
	backend.remove(name);
}

} // namespace discover
} // namespace astro
