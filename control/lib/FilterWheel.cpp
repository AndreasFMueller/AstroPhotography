/*
 * FilterWheel.cpp -- basic filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

using namespace astro::device;

namespace astro {
namespace camera {

DeviceName::device_type	FilterWheel::devicetype = DeviceName::Filterwheel;

DeviceName	FilterWheel::defaultname(const DeviceName& parent, const std::string& unitname) {
	return DeviceName(parent, DeviceName::Filterwheel, unitname);
}

FilterWheel::FilterWheel(const DeviceName& name) : Device(name) {
}

FilterWheel::FilterWheel(const std::string& name) : Device(name) {
}

FilterWheel::~FilterWheel() {
}

} // namespace camera
} // namespace astro
