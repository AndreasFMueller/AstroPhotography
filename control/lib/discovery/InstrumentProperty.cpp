/*
 * InstrumentProperty.cpp -- InstrumentProperty implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>
#include <AstroFormat.h>

namespace astro {
namespace discover {

/**
 * \brief Produce string representation of InstrumentProperty object
 */
std::string	InstrumentProperty::toString() const {
	std::string	common =  stringprintf("%s/%s = %s",
		_instrument.c_str(), _property.c_str(), _value.c_str());
	if (0 == _description.size()) {
		return common;
	}
	return common + " (" + _description + ")";
}

} // namespace discover
} // namespace astro
