/*
 * LuminanceFunctionFactory.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTonemapping.h>
#include <LuminanceFunctions.h>

namespace astro {
namespace adapter {

/**
 * \brief Get a LuminanceFunctionPtr by name
 *
 * \param name		name of the function to retrieve
 */
LuminanceFunctionPtr	LuminanceFunctionFactory::get(const std::string& name) {
	LuminanceFunction::parameters_t	parameters;
	return get(name, parameters);
}

/**
 * \brief Get a LuminanceFunctionPtr by name and parameters
 *
 * \param name		name of the function to retrieve
 * \param parameters	the parameters to use when constructing the function
 */
LuminanceFunctionPtr	LuminanceFunctionFactory::get(const std::string& name,
			const LuminanceFunction::parameters_t& parameters) {
	LuminanceFunction	*result = NULL;
	if (name == "asinh") {
		result = new AsinhFunction(parameters);
	}
	if (name == "atan") {
		result = new AtanFunction(parameters);
	}
	if (name == "atanh") {
		result = new AtanhFunction(parameters);
	}
	if (name == "gamma") {
		result = new GammaFunction(parameters);
	}
	if (name == "log") {
		result = new LogFunction(parameters);
	}
	if (NULL != result) {
		return LuminanceFunctionPtr(result);
	}
	std::string	msg = stringprintf("unknown function name '%s'",
		name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(name);
}

} // namespace adapter
} // namespace astro
