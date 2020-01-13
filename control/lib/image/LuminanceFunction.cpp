/*
 * LuminanceFunction.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTonemapping.h>

namespace astro {
namespace adapter {

/**
 * \brief Destroy the LuminanceFunction object
 */
LuminanceFunction::~LuminanceFunction() {
}

/**
 * \brief Default constructor of a LuminanceFunction
 */
LuminanceFunction::LuminanceFunction() {
	_x1 = 0;
	_x2 = 1;
	_y1 = 0;
	_y2 = 1;
	_use_absolute = false;
	_truncate_negative = false;
}

/**
 * \brief Converstion from string to double with appropriate error logging
 *
 * \param p	parameter pair
 */
static double	convert_to_double(const std::pair<std::string, std::string>& p) {
	try {
		return std::stod(p.second);
	} catch (const std::exception& ex) {
		std::string	msg = stringprintf("cannot convert "
			"'%s->%s': %s", p.first.c_str(),
			p.second.c_str(), ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw ex;
	}
}

/**
 * \brief Construct LuminanceFunction from parameter map
 *
 * \param parameters	parameter to construct basic parameters from
 */
LuminanceFunction::LuminanceFunction(
	const LuminanceFunction::parameters_t& parameters) {
	parameters_t::const_iterator	i = parameters.find("x1");
	if (i != parameters.end()) {
		_x1 = convert_to_double(*i);
	}
	i = parameters.find("x2");
	if (i != parameters.end()) {
		_x2 = convert_to_double(*i);
	}
	i = parameters.find("y1");
	if (i != parameters.end()) {
		_y1 = convert_to_double(*i);
	}
	i = parameters.find("y2");
	if (i != parameters.end()) {
		_y2 = convert_to_double(*i);
	}
	i = parameters.find("absolute");
	if (i != parameters.end()) {
		_use_absolute = ((i->second == "true") ||
			(i->second == "yes") || (i->second == "YES"));
	}
	i = parameters.find("truncate_negative");
	if (i != parameters.end()) {
		_truncate_negative = ((i->second == "true") ||
			(i->second == "yes") || (i->second == "YES"));
	}
}

/**
 * \brief Convert from the interval [x1,x2] to [0,1]
 *
 * \param	the parameter to convert
 */
double	LuminanceFunction::x(double l) const {
	double	xx = (l - x1()) / (x2() - x1());
	if ((truncate_negative()) && (xx < 0)) {
		return 0.;
	}
	if ((use_absolute()) && (xx < 0)) {
		return abs(xx);
	}
	return xx;
}

/**
 * \brief Convert from the interval [0,1] to [y1, y2]
 *
 * \param x	parameter to convert
 */
double	LuminanceFunction::y(double x) const {
	return y1() + (y2() - y1()) * x;
}

} // namespace adapter
} // namespace astro
