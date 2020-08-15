/*
 * SxUtils.cpp -- starlight express utilities 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <DeviceNameUSB.h>
#include "SxUtils.h"
#include "sx.h"
#include <AstroExceptions.h>
#include <SxCamera.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

size_t const SxName::number_sx_models = 40;

sx_model_t	SxName::models[] = {
// product, model,              name,           friendlyname
{ 0x0105, 0x0045, 		"m5",		"SXVF-M5",		true  },
{ 0x0305, 0x00c5, 		"m5c",		"SXVF-M5C",		true  },
{ 0x0107, 0x0047, 		"m7",		"SXVF-M7",		true  },
{ 0x0307, 0x00c7, 		"m7c",		"SXVF-M7C",		true  },
{ 0x0000, 0x0048, 		"m8",		"SXVF-M8",		true  },
{ 0x0308, 0x00c8, 		"m8c",		"SXVF-M8C",		true  },
{ 0x0109, 0x0049, 		"mx49",		"MX9",			true  },
{ 0x0109, 0x0000, 		"m9",		"SXVF-M9",		true  },
{ 0x0309, 0x00c9, 		"m9c",		"MX9C",			true  },
{ 0x0509, 0x0009, 		"oculus",	"Oculus",		true  },
{ 0x0325, 0x0059, 		"m25c",		"SXVR-M25C",		true  },
{ 0x0326, SX_MODEL_M26C,	"m26c",		"SXVR-M26C",		true  },
{ 0x0128, 0x0000, 		"h18",		"SXVR-H18",		true  },
{ 0x0126, 0x0000, 		"h16",		"SXVR-H16",		true  },
{ 0x0135, 0x0023, 		"h25",		"SXVR-H35",		true  },
{ 0x0135, 0x00b3, 		"h35c",		"SXVR-H35C",		true  },
{ 0x0136, 0x0024, 		"h36",		"SXVR-H36",		true  },
{ 0x0136, 0x00b4, 		"h36c",		"SXVR-H36C",		true  },
{ 0x0100, 0x0009, 		"h9",		"SXVR-H9",		true  },
{ 0x0119, 0x0009, 		"h9",		"SXVR-H9",		true  },
{ 0x0319, 0x0089, 		"h9c",		"SXVR-H9C",		true  },
{ 0x0100, 0x0089, 		"h9c",		"SXVR-H9C",		true  },
{ 0x0200, 0x0000, 		"interface",	"SXV interface",	false },
{ 0x0507, 0x0000, 		"lodestar",	"Lodestar",		false },
{ 0x0507, 0x0000, 		"lodestarc",	"Lodestar-C",		false },
{ 0x0517, 0x0000, 		"costar",	"CoStar",		false },
{ 0x0000, 0x0009, 		"hx9",		"HX9",			true  },
{ 0x0000, 0x0010, 		"h16",		"SXVR-H16",		true  },
{ 0x0000, 0x0090, 		"h16c",		"SXVR-H16C",		true  },
{ 0x0000, 0x0012, 		"h18",		"SXVR-H18",		true  },
{ 0x0000, 0x0092, 		"h18c",		"SXVR-H18C",		true  },
{ 0x0000, 0x0056, 		"h674",		"SXVR-H674",		true  },
{ 0x0000, 0x00b6, 		"h674c",	"SXVR-H674C",		true  },
{ 0x0000, 0x0057, 		"h694",		"SXVR-H694",		true  },
{ 0x0000, 0x00b7, 		"h694c",	"SXVR-H694C",		true  },
{ 0x0000, 0x0028, 		"h814",		"SXVR-H814",		true  },
{ 0x0000, 0x00a8, 		"h814c",	"SXVR-H814C",		true  },
{ 0x0000, 0x0058, 		"h290",		"SXVR-H290",		true  },
{ 0x0000, SX_MODEL_56,		"sx56",		"SX-56",		true  },
{ 0x0000, SX_MODEL_46,		"sx46",		"SX-46",		true  },
};

SxError::SxError(const char *cause) : std::runtime_error(cause) {
}

/**
 * \brief Construct an SxName from a USB device ptr
 */
SxName::SxName(DeviceName::device_type type, usb::DevicePtr deviceptr)
	: DeviceName(type, SX_MODULE_NAME, "blubb") {
	// first find out whether we can get a unique name from the
	// product id
	int	counter = 0;
	std::string	name;
	_product = deviceptr->getProductId();
	for (size_t i = 0; i < number_sx_models; i++) {
		if (_product == SxName::models[i].product) {
			counter++;
			name = SxName::models[i].name;
		}
	}

	// test whether the name is unique
	if (counter == 1) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found enclosure name '%s'",
			name.c_str());
		enclosurename(name);
	}

	// the product id did not uniquely determine the name, so we have
	// to query it for the model number
	_model = SxCamera::getModel(deviceptr);
	if (counter != 1) {
		enclosurename(deviceName(_product, _model));
	}
}

/**
 * \brief convert the command code into a printable name
 */
std::string	command_name(sx_command_t command) {
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
	case SX_CMD_READ_PIXELS_GATED:
		return std::string("read pixels gated");
	case SX_CMD_GET_BUILD_NUMBER:
		return std::string("get build number");
	case SX_CMD_COOLER:
		return std::string("cooler");
	case SX_CMD_COOLER_TEMPERATURE:
		return std::string("cooler temperature");
	case SX_CMD_SHUTTER:
		return std::string("shutter");
	case SX_CMD_READ_I2CPORT:
		return std::string("read i2cport");
	case SX_CMD_FLOOD_CCD:
		return std::string("flood ccd");
	}
	std::string	cmd = stringprintf("UNKNOWN CMD %d");
	return cmd;
}

/**
 * \brief Convert a wide string to a 8bit string
 *
 * \param w	wide character string to convert
 */
std::string     wchar2string(const wchar_t *w) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting string at %p", w);
	int	l = wcslen(w) + 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "string length is %d", l);
	if (l <= 1) {
		return std::string("");
	}
	char	buffer[l];
	int	i = 0;
	memset(buffer, 0, l);
	const wchar_t	*p = w;
	while (p) {
		int	r = wctob(*p);
		if (r != WEOF) {
			buffer[i++] = r;
		}
	}
        return std::string(buffer);
}

/**
 * \brief get the user friendly name
 *
 * \param product	the USB product id of the device
 * \param model		the ECHO2 model number
 */
std::string	SxName::userFriendlyName(unsigned short product,
			unsigned short model) {
	// try to match model and product
	for (size_t i = 0; i < number_sx_models; i++) {
		if ((product == SxName::models[i].product)
			&& (model == models[i].model)) {
			return std::string(SxName::models[i].friendlyname);
		}
	}

	// try to match model alone, at least if model != 0
	if (model != 0) {
		for (size_t i = 0; i < number_sx_models; i++) {
			if (model == SxName::models[i].model) {
				return std::string(SxName::models[i].friendlyname);
			}
		}
	}

	// try to match product alone
	if (product != 0) {
		for (size_t i = 0; i < number_sx_models; i++) {
			if (product == SxName::models[i].product) {
				return std::string(SxName::models[i].friendlyname);
			}
		}
	}

	// throw a exception signaling that we don't have a name
	std::string	msg = stringprintf("no name for product=%hu model=%hu",
		product, model);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw NotFound(msg);
}

/**
 * \brief get the user friendly name
 *
 * \param product	the USB product id of the device
 * \param model		the ECHO2 model number
 */
std::string	SxName::deviceName(unsigned short product,
			unsigned short model) {
	// try to match model and product
	for (size_t i = 0; i < number_sx_models; i++) {
		if ((product == SxName::models[i].product)
			&& (model == models[i].model)) {
			return std::string(SxName::models[i].name);
		}
	}

	// try to match model alone, at least if model != 0
	if (model != 0) {
		for (size_t i = 0; i < number_sx_models; i++) {
			if (model == SxName::models[i].model) {
				return std::string(SxName::models[i].name);
			}
		}
	}

	// try to match product alone
	if (product != 0) {
		for (size_t i = 0; i < number_sx_models; i++) {
			if (product == SxName::models[i].product) {
				return std::string(SxName::models[i].name);
			}
		}
	}

	// throw a exception signaling that we don't have a name
	std::string	msg = stringprintf("no name for product=%hu model=%hu",
		product, model);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw NotFound(msg);
}

/**
 * \brief whether or not the device has a cooler
 *
 * \param product	the USB product id of the device
 * \param model		the ECHO2 model number
 */
bool	SxName::hasCooler(unsigned short product,
			unsigned short model) {
	// try to match model and product
	for (size_t i = 0; i < number_sx_models; i++) {
		if ((product == SxName::models[i].product)
			&& (model == models[i].model)) {
			return SxName::models[i].hascooler;
		}
	}

	// try to match model alone, at least if model != 0
	if (model != 0) {
		for (size_t i = 0; i < number_sx_models; i++) {
			if (model == SxName::models[i].model) {
				return SxName::models[i].hascooler;
			}
		}
	}

	// try to match product alone
	if (product != 0) {
		for (size_t i = 0; i < number_sx_models; i++) {
			if (product == SxName::models[i].product) {
				return SxName::models[i].hascooler;
			}
		}
	}

	// throw a exception signaling that we don't have a name
	std::string	msg = stringprintf("no name for product=%hu model=%hu",
		product, model);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw NotFound(msg);
}

/**
 * \brief Construct the enclosure name from the device
 *
 * \param devptr	the USB device to use
 */
std::string	SxName::deviceName(usb::DevicePtr devptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing name for '%hx/%hx'",
		devptr->getVendorId(), devptr->getProductId());
	// first find out whether we can get a unique name from the
	// product id
	int	counter = 0;
	std::string	enclosurename;
	unsigned short	product = devptr->getProductId();
	for (size_t i = 0; i < number_sx_models; i++) {
		if (product == SxName::models[i].product) {
			counter++;
			enclosurename = SxName::models[i].name;
		}
	}

	// test whether the name is unique
	if (counter == 1) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found enclosure name '%s'",
			enclosurename.c_str());
		return enclosurename;
	}

	// the product id did not uniquely determine the name, so we have
	// to query it for the model number
	unsigned short	model = SxCamera::getModel(devptr);
	return deviceName(product, model);
}

DeviceName	SxName::cameraname(const DeviceName& other) {
	DeviceName	result(DeviceName::Camera, "sx", other.enclosurename());
	return result;
}

DeviceName	SxName::ccdname(const DeviceName& other) {
	DeviceName	result = cameraname(other);
	result.type(DeviceName::Ccd);
	result.push_back("Imager");
	return result;
}

DeviceName	SxName::coolername(const DeviceName& other) {
	DeviceName	result = ccdname(other);
	result.type(DeviceName::Cooler);
	result.push_back("cooler");
	return result;
}

DeviceName	SxName::guideportname(const DeviceName& other) {
	DeviceName	result = cameraname(other);
	result.push_back("guideport");
	result.type(DeviceName::Guideport);
	return result;
}

DeviceName	SxName::cameraname() const {
	return cameraname(*this);
}

DeviceName	SxName::ccdname() const {
	return ccdname(*this);
}

DeviceName	SxName::coolername() const {
	return coolername(*this);
}

DeviceName	SxName::guideportname() const {
	return guideportname(*this);
}

} // namespace sx
} // namespace camera
} // namespace astro

