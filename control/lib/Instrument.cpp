/*
 * Instrument.cpp -- Instrument class implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <InstrumentTables.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <sstream>

using namespace astro::persistence;

namespace astro {
namespace config {

//////////////////////////////////////////////////////////////////////
// Instrument Component methods 
//////////////////////////////////////////////////////////////////////

std::string	InstrumentComponent::type_name() const {
	return InstrumentComponentTableAdapter::type(_type);
}

std::string	InstrumentComponent::component_typename() const {
	return InstrumentComponentTableAdapter::component_type(_component_type);
}

std::string	InstrumentComponent::toString() {
	return stringprintf("%-16.16s %-8.8s %-40.40s  %ld",
		type_name().c_str(), component_typename().c_str(),
		name().c_str(), unit());
}

//////////////////////////////////////////////////////////////////////
// Instrument Component methods for direct components
//////////////////////////////////////////////////////////////////////

std::string     InstrumentComponentDirect::name() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "direct name: %s",
		_devicename.toString().c_str());
	return _devicename.toString();
}

//////////////////////////////////////////////////////////////////////
// Instrument Component methods for mapped components
//////////////////////////////////////////////////////////////////////

/**
 * \brief Get the device name for a mapped device
 */
DeviceName	InstrumentComponentMapped::devicename() {
	DeviceMapperPtr	devicemapper = DeviceMapper::get(_database);
	return devicemapper->find(_name).devicename();
}

/**
 * \brief Get the unit number for a mapped device
 */
int	InstrumentComponentMapped::unit() {
	DeviceMapperPtr	devicemapper = DeviceMapper::get(_database);
	return devicemapper->find(_name).unitid();
}

/**
 * \brief get the name
 */
std::string	InstrumentComponentMapped::name() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mapped name: %s", _name.c_str());
	return _name;
}

//////////////////////////////////////////////////////////////////////
// Instrument Component methods fro derived components
//////////////////////////////////////////////////////////////////////
/**
 * \brief Name of the parent device
 *
 * For derived components, this only returns the device name of the
 * parent device, it is the client's responsibilty to retrieve the
 * correct subdevice of the parent device.
 */
DeviceName	InstrumentComponentDerived::devicename() {
	return _instrument->devicename(_derivedfrom);
}

/**
 * \brief Use string encoding of derived from type as the name
 */
std::string	InstrumentComponentDerived::name() const {
	return InstrumentComponentTableAdapter::type(_derivedfrom);
}

//////////////////////////////////////////////////////////////////////
// Instrument methods
//////////////////////////////////////////////////////////////////////
/**
 * \brief Create a new Instrument
 */
Instrument::Instrument(Database db, const std::string& name)
	: _database(db), _name(name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument '%s' created",
		name.c_str());
}

/**
 * \brief Check whether the instrument has a device of a given type
 */
bool	Instrument::has(const DeviceName::device_type type) const {
	return (components.find(type) != components.end());
}

/**
 * \brief
 */
InstrumentComponentPtr	Instrument::component(DeviceName::device_type type) const {
	if (!has(type)) {
		throw std::runtime_error("no component of this type");
	}
	return components.find(type)->second;
}

/**
 * \brief Find the type of the device
 */
InstrumentComponent::component_t	Instrument::component_type(const DeviceName::device_type type) const {
	return component(type)->component_type();
}

/**
 * \brief Get the name of the device 
 *
 * This method can only work for a mapped device
 */
std::string	Instrument::name(DeviceName::device_type type) {
	return component(type)->name();
}

/**
 * \brief Get the device name for a mapped device
 */
DeviceName	Instrument::devicename(DeviceName::device_type type) {
	return component(type)->devicename();
}

/**
 * \brief Add an instrument component to an instrument
 */
void	Instrument::add(InstrumentComponentPtr component) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add component of type %s",
		InstrumentComponentTableAdapter::component_type(
			component->component_type()).c_str());
	if (has(component->type())) {
		components.erase(component->type());
	}
	components.insert(std::make_pair(component->type(), component));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "component added");
}

/**
 * \brief Remove an instrument component from an instrument
 */
void	Instrument::remove(DeviceName::device_type type) {
	components.erase(type);
}

/**
 * \brief Unit associated with a device type
 */
int	Instrument::unit(DeviceName::device_type type) {
	return component(type)->unit();
}

/**
 * \brief Convert the Instrument to a string version
 */
std::string	Instrument::toString() const {
	std::ostringstream	out;
	out << stringprintf("%-16.16s ", _name.c_str());
	std::list<DeviceName::device_type>      types = component_types();
	for (auto ptr = types.begin(); ptr != types.end(); ptr++) {
		if (ptr != types.begin()) {
			out << ",";
		}
		out << InstrumentComponentTableAdapter::type(*ptr);
	}
	return out.str();
}

/**
 * \brief Retrieve a list of device type codes 
 */
std::list<DeviceName::device_type>	Instrument::component_types() const {
	std::list<DeviceName::device_type>	result;
	for (auto ptr = components.begin(); ptr != components.end(); ptr++) {
		result.push_back(ptr->second->type());
	}
	return result;
}

} // namespace config
} // namespace astro
