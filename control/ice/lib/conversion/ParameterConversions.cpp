/*
 * ParameterConversions.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <types.h>
#include <AstroDevice.h>

namespace snowstar {

struct ParameterDescription	convert(const astro::device::ParameterDescription& parameter) {
	struct ParameterDescription	result;
	result.name = parameter.name();
	switch (parameter.type()) {
	case astro::device::ParameterDescription::boolean:
		result.type = ParameterBoolean;
		break;
	case astro::device::ParameterDescription::range:
		result.type = ParameterRange;
		result.from = parameter.from();
		result.to = parameter.to();
		break;
	case astro::device::ParameterDescription::sequence:
		result.type = ParameterSequence;
		result.from = parameter.from();
		result.to = parameter.to();
		result.step = parameter.step();
		break;
	case astro::device::ParameterDescription::floatset: {
		result.type = ParameterSetFloat;
		std::set<float>	f = parameter.floatValues();
		copy(f.begin(), f.end(),
			std::back_inserter(result.floatvalues));
		}
		break;
	case astro::device::ParameterDescription::stringset: {
		result.type = ParameterSetString;
		std::set<std::string>	f = parameter.stringValues();
		copy(f.begin(), f.end(),
			std::back_inserter(result.stringvalues));
		}
		break;
	}
	return result;
}

astro::device::ParameterDescription	convert(const ParameterDescription& parameter) {
	switch (parameter.type) {
	case ParameterBoolean:
		return astro::device::ParameterDescription(parameter.name);
	case ParameterRange:
		return astro::device::ParameterDescription(parameter.name,
			parameter.from, parameter.to);
	case ParameterSequence:
		return astro::device::ParameterDescription(parameter.name,
			parameter.from, parameter.to, parameter.step);
	case ParameterSetFloat:
		return astro::device::ParameterDescription(parameter.name,
			parameter.floatvalues);
	case ParameterSetString:
		return astro::device::ParameterDescription(parameter.name,
			parameter.stringvalues);
	}
	throw std::logic_error("parameter type unknonw");
}


} // namespace snowstar
