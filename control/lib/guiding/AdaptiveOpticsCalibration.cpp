/*
 * AdaptiveOpticsCalibration.cpp -- guider calibration
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
 * \brief Construct a new AdaptiveOpticsCalibration object
 *
 * The default calibration has all members set to zero, in particular,
 * it cannot be inverted, and it is not possible to compute corrections.
 */
AdaptiveOpticsCalibration::AdaptiveOpticsCalibration(const ControlDeviceName& name) : BasicCalibration(name) {
}

/**
 * \brief Construct a AdaptiveOpticsCalibration object from coefficient array
 */
AdaptiveOpticsCalibration::AdaptiveOpticsCalibration(const ControlDeviceName& name, const double coefficients[6])
	: BasicCalibration(name, coefficients) {
}

/**
 * \brief Construct a AdaptiveOpticsCalibration object from a basic calibration
 */
AdaptiveOpticsCalibration::AdaptiveOpticsCalibration(const BasicCalibration& other)
	: BasicCalibration(other) {
}

/**
 * \brief Assign the common fields from a Basic calibration
 */
AdaptiveOpticsCalibration&	AdaptiveOpticsCalibration::operator=(
				const BasicCalibration& other) {
	BasicCalibration::operator=(other);

	// that's all
	return *this;
}

} // namespace guiding
} // namespace astro
