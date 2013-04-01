/*
 * UVCDescriptors.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschuel Rappersil
 */
#include <AstroUVC.h>
#include <sstream>

using namespace astro::usb;

namespace astro {
namespace usb {
namespace uvc {


//////////////////////////////////////////////////////////////////////
// UVCDescriptor
//////////////////////////////////////////////////////////////////////

UVCDescriptor::UVCDescriptor(const Device& device, const void *data, int length)
	: USBDescriptor(device, data, length) {
	bdescriptorsubtype = uint8At(2);
}

UVCDescriptor::UVCDescriptor(const UVCDescriptor& other) : USBDescriptor(other) {
	bdescriptorsubtype = other.bdescriptorsubtype;
}

UVCDescriptor&	UVCDescriptor::operator=(const UVCDescriptor& other) {
	bdescriptorsubtype = other.bdescriptorsubtype;
	return *this;
}

uint8_t	UVCDescriptor::bDescriptorSubtype() const {
	return bdescriptorsubtype;
}

} // namespace uvc
} // namespace usb
} // namespace astro
