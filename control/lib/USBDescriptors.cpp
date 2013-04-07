/*
 * USBDescriptors.cpp -- base class for all standard USB descriptors
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>

namespace astro {
namespace usb {

/////////////////////////////////////////////////////////////////////
// common descriptor base class implementation
/////////////////////////////////////////////////////////////////////

Descriptor::Descriptor(Device& device, const std::string& extra)
	: dev(device), extra_descriptors(extra) {
}

Descriptor::Descriptor(const Device& device, const void *extra,
	int extra_length)
	: dev(device), extra_descriptors((const char *)extra, extra_length) {
}

Device&	Descriptor::device() { return dev; }
const Device&	Descriptor::device() const { return dev; }

const std::string&	Descriptor::extra() const {
	return extra_descriptors;
}

} // namespace usb
} // namespace astro
