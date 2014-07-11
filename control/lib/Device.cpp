/*
 * Device.cpp -- Device property interface
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>

namespace astro {
namespace device {

/**
 * \brief Construct a device
 */
Device::Device(const std::string& name) : Properties(name), _name(name) {
}

/**
 * \brief Construct a device from a device name
 */
Device::Device(const DeviceName& name) : Properties(name), _name(name) {
}

/**
 * \brief Destructor
 */
Device::~Device() {
}

} // namespace device
} // namespace astro
