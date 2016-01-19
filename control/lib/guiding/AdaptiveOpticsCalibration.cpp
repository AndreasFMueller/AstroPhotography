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
AdaptiveOpticsCalibration::AdaptiveOpticsCalibration() {
	calibrationtype(AO);
}

/**
 * \brief Construct a AdaptiveOpticsCalibration object from coefficient array
 */
AdaptiveOpticsCalibration::AdaptiveOpticsCalibration(const double coefficients[6])
	: BasicCalibration(coefficients) {
	calibrationtype(AO);
}

/**
 * \brief Construct a AdaptiveOpticsCalibration object from a basic calibration
 */
AdaptiveOpticsCalibration::AdaptiveOpticsCalibration(const BasicCalibration& other)
	: BasicCalibration(other) {
	calibrationtype(AO);
}

/**
 * \brief Assign the common fields from a Basic calibration
 */
AdaptiveOpticsCalibration&	AdaptiveOpticsCalibration::operator=(
				const BasicCalibration& other) {
	// copy the coefficients
	for (int i = 0; i < 6; i++) {
		a[i] = other.a[i];
	}

	// copy the points and copy the points from other
	clear();
	std::copy(other.begin(), other.end(), back_inserter(*this));

	// that's all
	return *this;
}

} // namespace guiding
} // namespace astro
