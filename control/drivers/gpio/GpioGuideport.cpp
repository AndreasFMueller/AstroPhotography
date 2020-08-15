/*
 * GpioGuideport.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GpioGuideport.h>
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

GpioGuideport::GpioGuideport(const std::string& devicename)
	: astro::camera::BasicGuideport(devicename) {
}

GpioGuideport::~GpioGuideport() {
}

void	GpioGuideport::do_activate(uint8_t active) {
	BasicGuideport::do_activate(active);
}

} // namespace gpio
} // namespace device
} // namespace astro
