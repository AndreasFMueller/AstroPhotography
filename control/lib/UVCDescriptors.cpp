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

uint8_t	UVCDescriptor::bDescriptorSubtype() const {
	return bdescriptorsubtype;
}

} // namespace uvc
} // namespace usb
} // namespace astro
