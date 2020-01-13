/*
 * LuminanceFunctions.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <LuminanceFunctions.h>

namespace astro {
namespace adapter {

/**
 * \brief Construct the GammaFunction object
 *
 * \param parameters	The parameters used for the construction
 */
GammaFunction::GammaFunction(const LuminanceFunction::parameters_t& parameters)
	: LuminanceFunction(parameters) {
	_gamma = 1.;
	LuminanceFunction::parameters_t::const_iterator	i
		= parameters.find("gamma");
	if (i != parameters.end()) {
		try {
			_gamma = std::stod(i->second);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "using gamma = %.3f",
				_gamma);
		} catch (const std::exception& ex) {
			std::string	msg = stringprintf("cannot convert "
				"gamma->%s: %s", i->second.c_str(), ex.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw ex;
		}
	}
	// by default, we have to truncate negative numbers
	i = parameters.find("truncate_negative");
	if (i == parameters.end()) {
		truncate_negative(true);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"GammaFunction: default truncate negative");
	}
}

/**
 * \brief The Gamma function 
 *
 * \param l	
 */
double	GammaFunction::operator()(double l) {
	return y(pow(x(l), _gamma));
}

/**
 * \brief Construct a inverse hyperbolic sine mapping
 *
 * \param parameters	The parameters used for the construction
 */
AsinhFunction::AsinhFunction(const LuminanceFunction::parameters_t& parameters)
	: LuminanceFunction(parameters) {
}

/**
 * \brief The asinh function implementation
 *
 * \param l	the argument for the asinh function
 */
double	AsinhFunction::operator()(double l) {
	return y(asinh(x(l)));
}

/**
 * \brief Construct a luminance function based on atan
 *
 * \param parameters	The parameteres used for construction
 */
AtanFunction::AtanFunction(const LuminanceFunction::parameters_t& parameters)
	: LuminanceFunction(parameters) {
}

/**
 * \brief The atan function implementation
 *
 * \param l	the argument for the atan function
 */
double	AtanFunction::operator()(double l) {
	return y(atan(x(l)) / (M_PI/2));
}

/**
 * \brief Construct a luminance function based on atanh
 *
 * \param parameters	The parameteres used for construction
 */
AtanhFunction::AtanhFunction(const LuminanceFunction::parameters_t& parameters)
	: LuminanceFunction(parameters) {
}

/**
 * \brief The atanh function implementation
 *
 * \param l	the argument for tha atanh function
 */
double	AtanhFunction::operator()(double l) {
	return y(atanh(x(l)));
}

/**
 * \brief Construct a luminance function based on log
 *
 * \param parameters	The parameteres used for construction
 */
LogFunction::LogFunction(const LuminanceFunction::parameters_t& parameters)
	: LuminanceFunction(parameters) {
	// by default, we have to truncate negative numbers
	LuminanceFunction::parameters_t::const_iterator	i
		= parameters.find("truncate_negative");
	if (i == parameters.end()) {
		truncate_negative(true);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"LogFunction: default truncate negative");
	}
}

/**
 * \brief The log function implementation
 *
 * \param l	the argument for the log function
 */
double	LogFunction::operator()(double l) {
	return y(log2(1. + x(l)));
}

} // namespace adapter
} // namespace astro

