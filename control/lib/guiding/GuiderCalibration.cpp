/*
 * GuiderCalibration.cpp -- guider calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <sstream>

namespace astro {
namespace guiding {

/**
 * \brief Construct a new GuiderCalibration object
 *
 * The default calibration has all members set to zero, in particular,
 * it cannot be inverted, and it is not possible to compute corrections.
 */
GuiderCalibration::GuiderCalibration(const ControlDeviceName& name)
	: BasicCalibration(name) {
}

/**
 * \brief Construct a GuiderCalibration object from coefficient array
 */
GuiderCalibration::GuiderCalibration(const ControlDeviceName& name,
	const double coefficients[6]) : BasicCalibration(name, coefficients) {
}

/**
 * \brief Construct a GuiderCalibration object from a basic calibration
 */
GuiderCalibration::GuiderCalibration(const BasicCalibration& other)
	: BasicCalibration(other) {
}

/**
 * \brief Assign the common fields from a Basic calibration
 */
GuiderCalibration&	GuiderCalibration::operator=(
				const BasicCalibration& other) {
	BasicCalibration::operator=(other);
	return *this;
}

} // namespace guiding
} // namespace astro
