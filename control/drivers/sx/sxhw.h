/*
 * sxhw.h -- common definitions for the SX driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _sx_h
#define _sx_h

#include <AstroUSB.h>

using namespace astro::usb;

namespace astro {
namespace sx {

#define SX_CMD_GET_FIRMWARE_VERSION	255
#define SX_CMD_ECHO			0
#define SX_CMD_CLEAR_PIXELS		1
#define SX_CMD_READ_PIXELS_DELAYED	2
#define SX_CMD_READ_PIXELS		3
#define SX_CMD_SET_TIMER		4
#define SX_CMD_GET_TIMER		5
#define SX_CMD_RESET			6
#define SX_CMD_SET_CCD_PARAMS		7
#define SX_CMD_GET_CCD_PARAMS		8
#define SX_CMD_SET_STAR2K		9
#define SX_CMD_WRITE_SERIAL_PORT	10
#define SX_CMD_READ_SERIAL_PORT		11
#define SX_CMD_SET_SERIAL		12
#define SX_CMD_GET_SERIAL		13
#define SX_CMD_CAMERA_MODEL		14
#define SX_CMD_LOAD_EEPROM		15
#define SX_CMD_COOLER			30

typedef struct sx_firmware_version_s {
	uint16_t	minor_version;	// least significant byte first
	uint16_t	major_version;	// least significant byte first
} __attribute__((packed)) sx_firmware_version_t;

typedef struct sx_read_pixels_s {
	uint16_t	x_offset;
	uint16_t	y_offset;
	uint16_t	width;
	uint16_t	height;
	uint8_t		x;
	uint8_t		y;
} __attribute__((packed)) sx_read_pixels_t;

typedef struct sx_read_pixels_delayed_s : public sx_read_pixels_t {
	uint32_t	delay;
} __attribute__((packed)) sx_read_pixels_delayed_t;

typedef struct sx_timer_s {
	uint32_t	timer;
} __attribute__((packed)) sx_timer_t;

typedef struct sx_ccd_params_s {
	uint8_t		hfront_porch;
	uint8_t		hback_porch;
	uint16_t	width;
	uint8_t		vfront_porch;
	uint8_t		vback_porch;
	uint16_t	height;
	uint16_t	pixel_uwidth;
	uint16_t	pixel_uheight;
	uint16_t	color;
	uint8_t		bits_per_pixel;
	uint8_t		num_serial_ports;
	uint8_t		extra_capabilities;
} __attribute__((packed)) sx_ccd_params_t;

typedef struct sx_camera_model_s {
	uint16_t	model;
} __attribute__((packed)) sx_camera_model_t;

class SxCamera {
	Device&	device;
	InterfacePtr	datainterface;
public:
	SxCamera(Device &device);
	sx_firmware_version_t	getVersion();
	std::string	getEcho(const std::string& data);
	void	clear(uint16_t ccdindex);
	FramePtr	getImage(uint16_t ccdindex,
		const sx_read_pixels_t& read);
	void	reset();
	sx_ccd_params_t	getCcdParams(uint16_t ccdindex);
	void	writeSerial(uint16_t serialport, const std::string& data);
	std::string	readSerial(uint16_t serialport);
	uint16_t	getModel();
	uint32_t	getTimer();
	void	setTimer(uint32_t timer);
};

} // namespace sx
} // namespace astro

#endif /* _sx_h */
