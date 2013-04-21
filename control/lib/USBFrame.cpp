/*
 * USBFrame.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <string>

using namespace astro::usb;

namespace astro {
namespace usb {

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

} // namespace usb
} // namespace astro
