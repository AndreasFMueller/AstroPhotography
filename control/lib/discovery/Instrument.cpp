/*
 * ServiceInstrument.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <InstrumentComponentTable.h>
#include <InstrumentPropertyTable.h>

namespace astro {
namespace discover {

//////////////////////////////////////////////////////////////////////
// Instrument implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Does the instrument have a component of this type
 */
bool	Instrument::has(InstrumentComponentKey::Type type) {
	return this->nComponentsOfType(type) > 0;
}

InstrumentComponent     Instrument::getAdaptiveOptics(int index) {
	return this->get(InstrumentComponentKey::AdaptiveOptics, index);
}

InstrumentComponent     Instrument::getCamera(int index) {
	return this->get(InstrumentComponentKey::Camera, index);
}

InstrumentComponent     Instrument::getCcd(int index) {
	return this->get(InstrumentComponentKey::CCD, index);
}

InstrumentComponent     Instrument::getCooler(int index) {
	return this->get(InstrumentComponentKey::Cooler, index);
}

InstrumentComponent     Instrument::getGuiderCcd(int index) {
	return this->get(InstrumentComponentKey::GuiderCCD, index);
}

InstrumentComponent     Instrument::getGuiderPort(int index) {
	return this->get(InstrumentComponentKey::GuiderPort, index);
}

InstrumentComponent     Instrument::getFilterWheel(int index) {
	return this->get(InstrumentComponentKey::FilterWheel, index);
}

InstrumentComponent     Instrument::getFocuser(int index) {
	return this->get(InstrumentComponentKey::Focuser, index);
}

InstrumentComponent     Instrument::getMount(int index) {
	return this->get(InstrumentComponentKey::Mount, index);
}

bool	Instrument::hasAdaptiveOptics() {
	return this->has(InstrumentComponentKey::AdaptiveOptics);
}

bool	Instrument::hasCamera() {
	return this->has(InstrumentComponentKey::Camera);
}

bool	Instrument::hasCcd() {
	return this->has(InstrumentComponentKey::CCD);
}

bool	Instrument::hasCooler() {
	return this->has(InstrumentComponentKey::Cooler);
}

bool	Instrument::hasGuiderCcd() {
	return this->has(InstrumentComponentKey::GuiderCCD);
}

bool	Instrument::hasGuiderPort() {
	return this->has(InstrumentComponentKey::GuiderPort);
}

bool	Instrument::hasFilterWheel() {
	return this->has(InstrumentComponentKey::FilterWheel);
}

bool	Instrument::hasFocuser() {
	return this->has(InstrumentComponentKey::Focuser);
}

bool	Instrument::hasMount() {
	return this->has(InstrumentComponentKey::Mount);
}

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
	add(result, InstrumentComponent::AdaptiveOptics);
	add(result, InstrumentComponent::Camera);
	add(result, InstrumentComponent::CCD);
	add(result, InstrumentComponent::Cooler);
	add(result, InstrumentComponent::GuiderCCD);
	add(result, InstrumentComponent::GuiderPort);
	add(result, InstrumentComponent::FilterWheel);
	add(result, InstrumentComponent::Focuser);
	add(result, InstrumentComponent::Mount);
	return result;
}

int	Instrument::getInt(const std::string& name) {
	return std::stoi(getProperty(name).value());
}

double	Instrument::getDouble(const std::string& name) {
	return std::stod(getProperty(name).value());
}

std::string	Instrument::getString(const std::string& name) {
	return getProperty(name).value();
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
	static InstrumentComponentTablePtr	components;
	static InstrumentPropertyTablePtr	properties;
	static void	setup() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setup of with default db");
		astro::config::ConfigurationPtr	config
			= astro::config::Configuration::get();
		database = config->database();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get handle to table");
		components = InstrumentComponentTablePtr(
				new InstrumentComponentTable(database));
		properties = InstrumentPropertyTablePtr(
				new InstrumentPropertyTable(database));
	}
	static void	setupdb(Database db) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setup with separate db");
		database = db;
		components = InstrumentComponentTablePtr(
				new InstrumentComponentTable(database));
		properties = InstrumentPropertyTablePtr(
				new InstrumentPropertyTable(database));
	}
	static std::once_flag	ready;
public:
	InstrumentBackendImpl() {
		std::call_once(ready, setup);
	}
	InstrumentBackendImpl(persistence::Database database) {
		std::call_once(ready, setupdb, database);
	}
	// instrument component methods
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
	int	indexOf(const std::string& instrumentname,
			InstrumentComponent::Type type,
			const std::string& devicename);
	// property methods
private:
	long	propertyid(const std::string& instrumentname,
				const std::string& property);
public:
	long	addProperty(const InstrumentProperty& property);
	bool	hasProperty(const std::string& instrumentname,
			const std::string& property);
	InstrumentProperty	getProperty(const std::string& instrumentname,
				const std::string& property);
	void	removeProperty(const std::string& instrumentname,
				const std::string& property);
	void	updateProperty(const InstrumentProperty& property);
	Instrument::PropertyNames	getPropertyNames(const std::string& instrumentname);
	InstrumentPropertyList	getProperties(const std::string& instrumentname);
};

astro::persistence::Database	InstrumentBackendImpl::database;
InstrumentComponentTablePtr	InstrumentBackendImpl::components;
InstrumentPropertyTablePtr	InstrumentBackendImpl::properties;
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
	long	id = components->add(record);
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
	components->update(objectid, info);
}

/**
 * \brief Remove a component
 */
void	InstrumentBackendImpl::remove(const std::string& name,
			InstrumentComponent::Type type, int index) {
	int	n = nComponentsOfType(name, type);

	// remove the component
	long	objectid = idfromkey(name, type, index);
	components->remove(objectid);
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

/**
 * \brief Remove an instrument
 */
void	InstrumentBackendImpl::remove(const std::string& name) {
	std::string	query(	"delete from instrumentcomponents "
				"where name = ?");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, name);
	statement->execute();
	query = std::string(	"delete from instrumentproperties "
				"where instrument = ?");
	statement = database->statement(query);
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
				"union "
				"select distinct instrument "
				"from instrumentproperties");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"request instrument %s, type=%d(%s), index=%d",
		name.c_str(), type,
		InstrumentComponentKey::type2string(type).c_str(), index);
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
	if (res.size() == 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "instrument %s: no matching "
			"component type=%d(%s), index=%d",
			name.c_str(), type,
			InstrumentComponentKey::type2string(type).c_str(),
			index);
		throw std::runtime_error("no matching instrument component");
	}
	long	objectid = res.front()[0]->intValue();
	return objectid;
}

/**
 * \brief Get the component
 */
InstrumentComponent	InstrumentBackendImpl::get(const std::string& name, 
				InstrumentComponent::Type type, int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"try to get instrument %s, component=%d(%s), index=%d", 
		name.c_str(), type,
		InstrumentComponentKey::type2string(type).c_str(), index);
	long	i;
	try {
		i = idfromkey(name, type, index);
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception");
		throw;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "id = %d", i);
	InstrumentComponentInfo	info = components->byid(i);
	InstrumentComponent	component(info, info.servicename(),
					info.deviceurl());
	return component;
}

/**
 * \brief Find the index of an instrument component by name
 */
int	InstrumentBackendImpl::indexOf(const std::string& instrumentname,
			InstrumentComponent::Type type,
			const std::string& deviceurl) {
	int	index;
	std::string	query(	"select idx "
				"from instrumentcomponents "
				"where name = ? "
				"  and type = ? "
				"  and deviceurl = ?");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, instrumentname);
	statement->bind(1, type);
	statement->bind(2, deviceurl);
	Result	res = statement->result();
	if (0 == res.size()) {
		std::string	cause = stringprintf("no instrument='%s' "
			"component='%s(%d)' deviceurl='%s'",
			instrumentname.c_str(),
			InstrumentComponentKey::type2string(type).c_str(),
			type, deviceurl.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	index = res.front()[0]->intValue();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"instrumentcomponent %s/%s(%d)/%s has index %d",
		instrumentname.c_str(),
		InstrumentComponentKey::type2string(type).c_str(), type,
		deviceurl.c_str(), index);
	return index;
}

/**
 * \brief Add a property to the database
 */
long	InstrumentBackendImpl::addProperty(const InstrumentProperty& property) {
	InstrumentPropertyRecord	record(property);
	long	id = properties->add(record);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new property with id = %ld", id);
	return id;
}

/**
 * \brief Common method to get the id of a property
 */
long	InstrumentBackendImpl::propertyid(const std::string& instrumentname,
		const std::string& property) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query instrument=%s, property=%s",
		instrumentname.c_str(), property.c_str());
	std::string	query(	"select id "
				"from instrumentproperties "
				"where instrument = ? "
				"  and property = ? ");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, instrumentname);
	statement->bind(1, property);
	Result	res = statement->result();
	if (0 == res.size()) {
		std::string	cause = stringprintf(
			"no property instrument='%s' property='%s'",
			instrumentname.c_str(), property.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "property not found: %s",
			cause.c_str());
		throw std::runtime_error(cause);
	}
	int	_propertyid = res.front()[0]->intValue();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "property %s/%s has id %d",
		instrumentname.c_str(), property.c_str(), _propertyid);
	return _propertyid;
}

/**
 * \brief Retrieve a property object from the database
 */
InstrumentProperty	InstrumentBackendImpl::getProperty(
				const std::string& instrumentname,
				const std::string& property) {
	int	_propertyid = propertyid(instrumentname, property);
	InstrumentProperty	p = properties->byid(_propertyid);
	return p;
}

bool	InstrumentBackendImpl::hasProperty(const std::string& instrumentname,
		const std::string& property) {
	try {
		propertyid(instrumentname, property);
		return true;
	} catch (const std::exception& x) {
	}
	return false;
}

void	InstrumentBackendImpl::removeProperty(
				const std::string& instrumentname,
				const std::string& property) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove property %s from instrument %s",
		property.c_str(), instrumentname.c_str());
	std::string	query(	"delete from instrumentproperties "
				"where instrument = ? "
				"  and property = ?");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, instrumentname);
	statement->bind(1, property);
	statement->execute();
}

void	InstrumentBackendImpl::updateProperty(
		const InstrumentProperty& property) {
	int	_propertyid = propertyid(property.instrument(),
			property.property());
	InstrumentProperty	p = properties->byid(_propertyid);
	p.value(property.value());
	p.description(property.description());
	properties->update(_propertyid, p);
}

InstrumentPropertyList	InstrumentBackendImpl::getProperties(
					const std::string& instrumentname) {
	InstrumentPropertyList	properties;
	std::string	query(	"select property "
				"from instrumentproperties "
				"where instrument = ? "
				"order by 1");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, instrumentname);
	Result	res = statement->result();
	Result::const_iterator	i;
	for (i = res.begin(); i != res.end(); i++) {
		std::string	property = (*i)[0]->stringValue();
		properties.push_back(getProperty(instrumentname, property));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d property names",
		properties.size());
	return properties;
}

Instrument::PropertyNames	InstrumentBackendImpl::getPropertyNames(
					const std::string& instrumentname) {
	Instrument::PropertyNames	names;
	std::string	query(	"select property "
				"from instrumentproperties "
				"where instrument = ? "
				"order by 1");
	StatementPtr	statement = database->statement(query);
	statement->bind(0, instrumentname);
	Result	res = statement->result();
	Result::const_iterator	i;
	for (i = res.begin(); i != res.end(); i++) {
		std::string	name = (*i)[0]->stringValue();
		names.push_back(name);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d property names",
		names.size());
	return names;
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
	virtual int	indexOf(InstrumentComponent::Type type,
				const std::string& deviceurl) {
		return backend.indexOf(name(), type, deviceurl);
	}
	// property methods
	virtual int	addProperty(const InstrumentProperty& property) {
		return backend.addProperty(property);
	}
	virtual InstrumentProperty	getProperty(const std::string& property) {
		return backend.getProperty(name(), property);
	}
	virtual bool	hasProperty(const std::string& property) {
		return backend.hasProperty(name(), property);
	}
	virtual void	removeProperty(const std::string& property) {
		backend.removeProperty(name(), property);
	}
	virtual void	updateProperty(const InstrumentProperty& property) {
		backend.updateProperty(property);
	}
	virtual PropertyNames	getPropertyNames() {
		return backend.getPropertyNames(name());
	}
	virtual InstrumentPropertyList	getProperties() {
		return backend.getProperties(name());
	}
};

InstrumentPtr	InstrumentBackendImpl::get(const std::string& name) {
	InstrumentImpl	*instrument = new InstrumentImpl(name);
	return InstrumentPtr(instrument);
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
