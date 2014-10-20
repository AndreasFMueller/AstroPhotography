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

namespace astro {
namespace device {
namespace gpio {

GpioGuiderport::GpioGuiderport(const std::string& devicename)
	: astro::camera::GuiderPort(devicename) {
}

GpioGuiderport::~GpioGuiderport() {
}

uint8_t	GpioGuiderport::active() {
}

void	GpioGuiderport::activate(float raplus, float raminus,
		float decplus, float decminus) {
}

} // namespace gpio
} // namespace device
} // namespace astro
