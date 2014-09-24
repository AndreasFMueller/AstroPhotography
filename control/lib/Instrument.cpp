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
using namespace astro::module;
using namespace astro::camera;

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
	return stringprintf("%-16.16s %-8.8s %-32.32s  %-2ld %s",
		type_name().c_str(), component_typename().c_str(),
		name().c_str(), unit(), servername().c_str());
}

//////////////////////////////////////////////////////////////////////
// Instrument Component methods for direct components
//////////////////////////////////////////////////////////////////////

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
 * \brief Try to change the unit number in a mapped device
 */
void	InstrumentComponentMapped::unit(int /* u */) {
	throw std::runtime_error("cannot change unit for mapped component, use device mapper to change unit id");
}

/**
 * \brief get the name
 */
std::string	InstrumentComponentMapped::name() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mapped name: %s", _name.c_str());
	return _name;
}

void	InstrumentComponentMapped::name(const std::string& n) {
	_name = n;
}

std::string	InstrumentComponentMapped::servername() {
	DeviceMapperPtr	devicemapper = DeviceMapper::get(_database);
	return devicemapper->find(_name).servername();
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

void	InstrumentComponentDerived::name(const std::string& n) {
	_derivedfrom = InstrumentComponentTableAdapter::type(n);
}

std::string	InstrumentComponentDerived::servername() {
	return _instrument->servername(_derivedfrom);
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
 * \brief Check whether an instrument component is local
 */
bool	Instrument::isLocal(const DeviceName::device_type type) const {
	if (!has(type)) {
		throw std::runtime_error("no component of this type");
	}
	return (component(type)->servername().size() == 0);
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
 * \brief Get the server name on which the device runs
 */
std::string	Instrument::servername(DeviceName::device_type type) {
	return component(type)->servername();
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

/**
 * \brief Get an adaptive optics unit from an instrument
 */
AdaptiveOpticsPtr	Instrument::adaptiveoptics() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve AO for instrument '%s'",
		_name.c_str());
	Repository	repository;
	Devices	devices(repository);
	InstrumentComponentPtr	aoptr = component(DeviceName::AdaptiveOptics);
	switch (aoptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		return devices.getAdaptiveOptics(aoptr->devicename());
	case InstrumentComponent::derived:
		throw std::runtime_error("don't know how to derive camera");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve AO for instrument '%s'",
		_name.c_str());
}

/**
 * \brief Get a camera from an instrument
 */
CameraPtr	Instrument::camera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve camera for instrument '%s'",
		_name.c_str());
	Repository	repository;
	Devices	devices(repository);
	InstrumentComponentPtr	cameraptr = component(DeviceName::Camera);
	switch (cameraptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		return devices.getCamera(cameraptr->devicename());
	case InstrumentComponent::derived:
		throw std::runtime_error("don't know how to derive camera");
	}
}

/**
 * \brief Get a CCD from an instrument
 */
CcdPtr	Instrument::ccd() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve CCD for instrument '%s'",
		_name.c_str());
	Repository	repository;
	Devices	devices(repository);
	InstrumentComponentPtr	ccdptr = component(DeviceName::Ccd);
	switch (ccdptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		return devices.getCcd(ccdptr->devicename());
	case InstrumentComponent::derived:
		break;
	}
	// if we get to this point, then we have to derive id
	InstrumentComponentDerived	*from
		= dynamic_cast<InstrumentComponentDerived *>(&*ccdptr);
	if (from->derivedfrom() != DeviceName::Camera) {
		throw std::runtime_error("only know how to derive from a camera");
	}
	return camera()->getCcd(ccdptr->unit());
}

/**
 * \brief Get a cooler from an instrument
 */
CoolerPtr	Instrument::cooler() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Cooler for instrument '%s'",
		_name.c_str());
	Repository	repository;
	Devices	devices(repository);
	InstrumentComponentPtr	coolerptr = component(DeviceName::Cooler);
	switch (coolerptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		return devices.getCooler(coolerptr->devicename());
	case InstrumentComponent::derived:
		break;
	}
	InstrumentComponentDerived	*from
		= dynamic_cast<InstrumentComponentDerived *>(&*coolerptr);
	if (from->derivedfrom() != DeviceName::Ccd) {
		throw std::runtime_error("only know how to derive from a ccd");
	}
	return ccd()->getCooler();
}

/**
 * \brief get a Filterwheel from an instrument
 */
FilterWheelPtr	Instrument::filterwheel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve FilterWheel for instrument '%s'",
		_name.c_str());
	Repository	repository;
	Devices	devices(repository);
	InstrumentComponentPtr	filterwheelptr
		= component(DeviceName::Filterwheel);
	switch (filterwheelptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		return devices.getFilterWheel(filterwheelptr->devicename());
	case InstrumentComponent::derived:
		break;
	}
	// if we get to this point, then we have to derive id
	InstrumentComponentDerived	*from
		= dynamic_cast<InstrumentComponentDerived *>(&*filterwheelptr);
	if (from->derivedfrom() != DeviceName::Camera) {
		throw std::runtime_error("only know how to derive from a camera");
	}
	return camera()->getFilterWheel();
}

/**
 * \brief get the Focuser for an instrument
 */
FocuserPtr	Instrument::focuser() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Focuser for instrument '%s'",
		_name.c_str());
	Repository	repository;
	Devices	devices(repository);
	InstrumentComponentPtr	focuserptr = component(DeviceName::Focuser);
	switch (focuserptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		return devices.getFocuser(focuserptr->devicename());
	case InstrumentComponent::derived:
		throw std::runtime_error("don't know how to derived Focuser");
	}
}

/**
 * \brief get a Mount from an instrument
 */
MountPtr	Instrument::mount() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve Mount for instrument '%s'",
		_name.c_str());
	Repository	repository;
	Devices	devices(repository);
	InstrumentComponentPtr	mountptr = component(DeviceName::Mount);
	switch (mountptr->component_type()) {
	case InstrumentComponent::direct:
	case InstrumentComponent::mapped:
		return devices.getMount(mountptr->devicename());
	case InstrumentComponent::derived:
		throw std::runtime_error("don't know how to derive mount");
	}
}


} // namespace config
} // namespace astro
