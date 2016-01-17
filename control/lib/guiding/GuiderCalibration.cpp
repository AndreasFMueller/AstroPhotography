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
GuiderCalibration::GuiderCalibration() {
	focallength = 0;
	masPerPixel = 0;
}

/**
 * \brief Construct a GuiderCalibration object from coefficient array
 */
GuiderCalibration::GuiderCalibration(const double coefficients[6])
	: BasicCalibration(coefficients) {
	focallength = 0;
	masPerPixel = 0;
}

/**
 * \brief Construct a GuiderCalibration object from a basic calibration
 */
GuiderCalibration::GuiderCalibration(const BasicCalibration& other)
	: BasicCalibration(other) {
	focallength = 0;
	masPerPixel = 0;
}

/**
 * \brief Assign the common fields from a Basic calibration
 */
GuiderCalibration&	GuiderCalibration::operator=(
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
