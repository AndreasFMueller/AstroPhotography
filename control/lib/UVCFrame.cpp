/*
 * UVCFrame.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <string>

using namespace astro::usb;
using namespace astro::usb::uvc;

namespace astro {
namespace usb {
namespace uvc {

Frame::Frame(const void *data, int length) : std::string((char *)data, length) {
}

int	Frame::getWidth() const {
	return width;
}

int	Frame::getHeight() const {
	return height;
}

} // namespace uvc
} // namespace usb
} // namespace astro
