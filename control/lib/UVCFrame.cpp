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

Frame::Frame(int _width, int _height) : width(_width), height(_height) {
}

Frame::Frame(int _width, int _height, void *data, size_t length)
	: std::string((char *)data, length), width(_width), height(_height) {
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
