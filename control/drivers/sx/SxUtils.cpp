/*
 * SxUtils.cpp -- starlight express utilities 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <DeviceNameUSB.h>
#include <sx.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

SxError::SxError(const char *cause) : std::runtime_error(cause) {
}

#define SX_MODULE_NAME	"sx"
#define SX_VENDOR_ID	0x1278

/**
 * \brief Construct an SxName from a USB device ptr
 */
SxName::SxName(usb::DevicePtr deviceptr)
	: DeviceNameUSB(SX_MODULE_NAME, SX_VENDOR_ID, deviceptr) {
}

/**
 * \brief Construct an SxName from a DeviceName
 */
SxName::SxName(const DeviceName& devicename)
	: DeviceNameUSB(SX_MODULE_NAME, SX_VENDOR_ID, devicename) {
}

/**
 * \brief convert the command code into a printable name
 */
std::string	command_name(int command) {
	switch (command) {
	case SX_CMD_GET_FIRMWARE_VERSION:
		return std::string("get firmware");
	case SX_CMD_ECHO:
		return std::string("echo");
	case SX_CMD_CLEAR_PIXELS:
		return std::string("clear pixels");
	case SX_CMD_READ_PIXELS_DELAYED:
		return std::string("read pixels delayed");
	case SX_CMD_READ_PIXELS:
		return std::string("read pixels");
	case SX_CMD_SET_TIMER:
		return std::string("set timer");
	case SX_CMD_GET_TIMER:
		return std::string("get timer");
	case SX_CMD_RESET:
		return std::string("reset");
	case SX_CMD_SET_CCD_PARAMS:
		return std::string("set ccd params");
	case SX_CMD_GET_CCD_PARAMS:
		return std::string("get ccd params");
	case SX_CMD_SET_STAR2K:
		return std::string("set star2k");
	case SX_CMD_WRITE_SERIAL_PORT:
		return std::string("write serial port");
	case SX_CMD_READ_SERIAL_PORT:
		return std::string("read serial port");
	case SX_CMD_SET_SERIAL:
		return std::string("set serial");
	case SX_CMD_GET_SERIAL:
		return std::string("get serial");
	case SX_CMD_CAMERA_MODEL:
		return std::string("camera model");
	case SX_CMD_LOAD_EEPROM:
		return std::string("load eeprom");
	case SX_CMD_COOLER:
		return std::string("cooler");
	}
	std::string	cmd = stringprintf("UNKNOWN CMD %d");
	return cmd;
}

} // namespace sx
} // namespace camera
} // namespace astro
