/*
 * InstrumentComponent.cpp -- component of instruments
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ServiceDiscovery.h>

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

} // namespace discover
} // namespace astro
