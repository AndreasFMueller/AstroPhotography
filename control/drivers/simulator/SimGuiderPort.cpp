/*
 * SimGuiderPort.cpp -- Guider Port implementation for simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimGuiderPort.h>

namespace astro {
namespace camera {
namespace simulator {

SimGuiderPort::SimGuiderPort(SimLocator& locator)
	: GuiderPort("sim-guiderport"), _locator(locator) {
}

} // namespace simulator
} // namespace camera
} // namespace astro
