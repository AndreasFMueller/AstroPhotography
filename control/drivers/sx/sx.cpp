/*
 * sx.cpp -- basic starlight express device
 *
 * (c) 2013 Prof Dr Andreas Mueller, 
 */
#include <sx.h>
#include <debug.h>

namespace astro {
namespace sx {

SxCamera::SxCamera(Device& _device) : device(_device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SxCamera");
}

sx_firmware_version_t	SxCamera::getVersion() {
}

std::string	SxCamera::getEcho(const std::string& data) {
	std::string	result;
	return result;
}

void	SxCamera::clear() {
}

FramePtr	SxCamera::getImage(const read_pixels_t& read_pixels) {
	FramePtr	ptr(new Frame(1,1));
	return ptr;
}

void	SxCamera::reset() {
}

ccd_params_t	SxCamera::getCcdParams(uint16_t ccd) {
}

void	SxCamera::writeSerial(const std::string& data) {
}

std::string	SxCamera::readSerial() {
	std::string	result;
	return result;
}

uint16_t	SxCamera::getModel() {
}

} // namespace sx
} // namespace astro
