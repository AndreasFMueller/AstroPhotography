/*
 * Device.cpp -- Device property interface
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <stdexcept>

namespace astro {
namespace device {

/**
 * \brief Construct a device
 */
Device::Device(const std::string& devname, DeviceName::device_type type)
	: Properties(devname), _name(devname) {
	if (name().type() != type) {
		throw std::runtime_error("name type does not match");
	}
}

/**
 * \brief Construct a device from a device name
 */
Device::Device(const DeviceName& devname, DeviceName::device_type type)
	: Properties(devname), _name(devname) {
	if (name().type() != type) {
		throw std::runtime_error("name type does not match");
	}
}

/**
 * \brief Destructor
 */
Device::~Device() {
}

} // namespace device
} // namespace astro
