/*
 * Calibration.cpp -- initialization of the calibration object
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CalibrationPersistence.h>
#include <includes.h>

namespace astro {
namespace guiding {

Calibration::Calibration() {
	time(&when);
	for (int i = 0; i < 6; i++) { a[i] = 0.; }
	quality = 0;
	det = 0;
	focallength = 0;
	complete = false;
}

} // namespace guiding
} // namespace astro
