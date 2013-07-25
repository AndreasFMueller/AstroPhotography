/*
 * sxhw.cpp -- basic starlight express device
 *
 * (c) 2013 Prof Dr Andreas Mueller, 
 */
#include <sxhw.h>
#include <AstroDebug.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

SxCamera::SxCamera(Device& _device) : device(_device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SxCamera");
	datainterface = (*device.activeConfig())[0];
}

sx_firmware_version_t	SxCamera::getVersion() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing version request");
	Request<sx_firmware_version_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_GET_FIRMWARE_VERSION, (uint16_t)0);
	device.controlRequest(&request);
	return *request.data();
}

std::string	SxCamera::getEcho(const std::string& data) {
	std::string	result;
	return result;
}

void	SxCamera::clear(uint16_t ccdindex) {
	EmptyRequest	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, ccdindex,
		(uint8_t)SX_CMD_CLEAR_PIXELS, (uint16_t)0);
	device.controlRequest(&request);
}

FramePtr	SxCamera::getImage(const uint16_t ccdindex,
	const sx_read_pixels_t& read_pixels) {
	FramePtr	ptr(new Frame(1,1));
	return ptr;
}

void	SxCamera::reset() {
	EmptyRequest	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_RESET, (uint16_t)0);
	device.controlRequest(&request);
}

sx_ccd_params_t	SxCamera::getCcdParams(uint16_t ccdindex) {
	Request<sx_ccd_params_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, ccdindex,
		(uint8_t)SX_CMD_GET_CCD_PARAMS, (uint16_t)0);
	device.controlRequest(&request);
	return *request.data();
}

void	SxCamera::writeSerial(uint16_t serialport, const std::string& data) {
}

std::string	SxCamera::readSerial(uint16_t serialport) {
	std::string	result;
	return result;
}

uint16_t	SxCamera::getModel() {
	Request<sx_camera_model_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_CAMERA_MODEL, (uint16_t)0);
	device.controlRequest(&request);
	return request.data()->model;
}

uint32_t	SxCamera::getTimer() {
	Request<sx_timer_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_SET_TIMER, (uint16_t)0);
	device.controlRequest(&request);
	return request.data()->timer;
}

void	SxCamera::setTimer(uint32_t timer) {
	sx_timer_t	t;
	t.timer = timer;
	Request<sx_timer_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_SET_TIMER, (uint16_t)0, &t);
	device.controlRequest(&request);
}

} // namespace sx
} // namespace camera
} // namespace astro
