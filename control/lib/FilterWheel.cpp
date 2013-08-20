/*
 * FilterWheel.cpp -- basic filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

using namespace astro::device;

namespace astro {
namespace camera {

FilterWheel::FilterWheel(const std::string name) : Device(name) {
}

FilterWheel::~FilterWheel() {
}

} // namespace camera
} // namespace astro
