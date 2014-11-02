/*
 * GpioGuiderport.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GpioGuiderport.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <includes.h>
#include <stdexcept>
#include <AstroDevice.h>
#include <sstream>

namespace astro {
namespace device {
namespace gpio {

GpioGuiderport::GpioGuiderport(const std::string& devicename)
	: astro::camera::BasicGuiderport(devicename) {
	start();
}

GpioGuiderport::~GpioGuiderport() {
}

void	GpioGuiderport::do_activate(uint8_t active) {
	BasicGuiderport::do_activate(active);
}

} // namespace gpio
} // namespace device
} // namespace astro
