/*
 * InstrumentComponent.cpp -- component of instruments
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>
#include <AstroFormat.h>
#include <Nice.h>

namespace astro {
namespace discover {

/**
 * \brief Constructor for Instrument components
 */
InstrumentComponent::InstrumentComponent(const std::string& instrumentname,
	InstrumentComponentKey::Type type,
	const std::string& servicename, const std::string& deviceurl)
	: InstrumentComponentKey(instrumentname, type),
	  _servicename(servicename), _deviceurl(deviceurl) {
}

InstrumentComponent::InstrumentComponent(const InstrumentComponentKey& key,
	const std::string& servicename, const std::string& deviceurl)
	: InstrumentComponentKey(key),
	  _servicename(servicename), _deviceurl(deviceurl) {
}

std::string	InstrumentComponent::toString() const {
	return stringprintf("%s: @%s %s",
		InstrumentComponentKey::type2string(type()).c_str(),
		servicename().c_str(), deviceurl().c_str());
}

DeviceName	InstrumentComponent::localizedName() const {
	// find out whether the name is local or uses the nice module
	bool	useNice = true;

	// if this is already a local name, used unchanged
	if (!useNice) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using localized name %s",
			_deviceurl.c_str());
		return DeviceName(_deviceurl);
	}

	// this is a remote name, so construct a remote name
	device::nice::DeviceNicer	nicer(_servicename);
	DeviceName	result = nicer(_deviceurl);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "localized name: %s",
		result.toString().c_str());
	return result;
}

} // namespace discover
} // namespace astro
