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
InstrumentComponent::InstrumentComponent(Type type,
	const std::string& servicename, const std::string& deviceurl)
	: _type(type), _index(-1), _servicename(servicename),
	  _deviceurl(deviceurl) {
}

} // namespace discover
} // namespace astro
