/*
 * InstrumentBackendImpl.cpp -- instrument implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "InstrumentBackendImpl.h"
#include "InstrumentImpl.h"
#include <AstroDiscovery.h>
#include <AstroPersistence.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <InstrumentComponentTable.h>
#include <InstrumentPropertyTable.h>

namespace astro {
namespace discover {

astro::persistence::Database	InstrumentBackendImpl::database;
InstrumentComponentTablePtr	InstrumentBackendImpl::components;
InstrumentPropertyTablePtr	InstrumentBackendImpl::properties;
std::once_flag	InstrumentBackendImpl::ready;

/**
 * \brief Count the number of components of a given type in an instrument
 *
 * \param name		name of the instrument
 * \param type		component type
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
 *
 * An instrument can have at most one component of type GuiderCCD, GuidePort
 * and AdaptiveOptics. This is enforced by the add method
 *
 * \param component	the component to add
 */
int	InstrumentBackendImpl::add(const InstrumentComponent& component) {
	std::string	instrumentname = component.name();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add component to instrument %s",
		instrumentname.c_str());

	// make sure there is at mostone guiderccd, guideport or ao
	switch (component.type()) {
	case InstrumentComponentKey::GuiderCCD:
	case InstrumentComponentKey::AdaptiveOptics:
	case InstrumentComponentKey::GuidePort:
	case InstrumentComponentKey::Mount:
		if (1 <= nComponentsOfType(instrumentname, component.type())) {
			std::string	typestring
				= InstrumentComponentKey::type2string(
					component.type());
			std::string	msg = stringprintf("instrument already "
				"has component of type %s", typestring.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		break;
	default:
		// ignore all the other types
		break;
	}

	// build a record and add it to the database
	InstrumentComponentRecord	record(component);
	record.name(component.name());
	record.index(nComponentsOfType(component.name(), component.type()));
	long	id = components->add(record);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new entry with id = %ld", id);
	return record.index();;
}

/**
 * \brief Update a component
 *
 * Update the component in the database from the information in the supplied
 * component.
 *
 * \param component	the component data to update in the database
 */
void	InstrumentBackendImpl::update(const InstrumentComponent& component) {
	long	objectid = idfromkey(component.name(), component.type(),
				component.index());
	InstrumentComponentInfo	info(component);
	components->update(objectid, info);
}

/**
 * \brief Remove a component
 *
 * \param name		then name of the instrument
 * \param type		type of the component
 * \param index		index of the component
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
 *
 * \param name	name of the instrument
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
 *
 * \param name		name of the instrument to locate
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
 *
 * \param key	the component key
 */
long	InstrumentBackendImpl::idfromkey(const InstrumentComponentKey& key) {
	return idfromkey(key.name(), key.type(), key.index());
}

/**
 * \brief Retrieve the object id from name, type and index of a component
 *
 * \param name		the name of the instrument
 * \param type		the type of the component
 * \param index		the index of the component
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
 *
 * \param name		name of the instrument
 * \param type		type of the component
 * \param index		index of the component
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
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception: %s", x.what());
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
 *
 * \param instrumentname	name of the instrument
 * \param type			type of the component
 * \param deviceurl		the device url of the component
 * \return the index of the component
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
 *
 * \param property	the property to add
 */
long	InstrumentBackendImpl::addProperty(const InstrumentProperty& property) {
	InstrumentPropertyRecord	record(property);
	long	id = properties->add(record);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new property with id = %ld", id);
	return id;
}

/**
 * \brief Common method to get the id of a property
 *
 * \param instrmentname		the name of the instrument
 * \param property		the property 
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
 *
 * \param instrumentname	the name of the instrument
 * \param property		the property name
 */
InstrumentProperty	InstrumentBackendImpl::getProperty(
				const std::string& instrumentname,
				const std::string& property) {
	int	_propertyid = propertyid(instrumentname, property);
	InstrumentProperty	p = properties->byid(_propertyid);
	return p;
}

/**
 * \brief Find out whether a property is present
 *
 * \param instrumentname	name of the instrument
 * \param property		the name of the property
 */
bool	InstrumentBackendImpl::hasProperty(const std::string& instrumentname,
		const std::string& property) {
	try {
		propertyid(instrumentname, property);
		return true;
	} catch (const std::exception& x) {
	}
	return false;
}

/**
 * \brief Remove a property from an instrument
 *
 * \param instrumentname	name of the instrument
 * \param property		name of the property to remove
 */
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

/**
 * \brief Update the property info in the database from a property object
 *
 * \param property	the instrument property to copy
 */
void	InstrumentBackendImpl::updateProperty(
		const InstrumentProperty& property) {
	int	_propertyid = propertyid(property.instrument(),
			property.property());
	InstrumentProperty	p = properties->byid(_propertyid);
	p.value(property.value());
	p.description(property.description());
	properties->update(_propertyid, p);
}

/**
 * \brief Retrieve a list of all properties of an instrument
 *
 * \param instrumentname	name of the instrument
 */
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

/**
 * \brief Retrieve a list of property names present in an instrument
 *
 * \param instrumentname	the name of the instrument
 */
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

/**
 * \brief Get an INstrument by name
 *
 * \param instrumentname		name of the instrument
 */
InstrumentPtr   InstrumentBackendImpl::get(const std::string& instrumentname) {
	InstrumentImpl  *instrument = new InstrumentImpl(instrumentname);
	return InstrumentPtr(instrument);
}

} // namespace discover
} // namespace astro
