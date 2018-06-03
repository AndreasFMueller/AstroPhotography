/*
 * sx.h -- common definitions for the SX driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _sx_h
#define _sx_h

#include <AstroUSB.h>

using namespace astro::usb;

#define	DEFAULT_AS_USB_CONTROL_REQUEST	true

#define	SX_VENDOR_ID	0x1278
#define SX_FILTERWHEEL_PRODUCT_ID	0x0000

namespace astro {
namespace camera {
namespace sx {

typedef enum sx_command_e {
	SX_CMD_GET_FIRMWARE_VERSION	= 255,
	SX_CMD_ECHO			= 0,
	SX_CMD_CLEAR_PIXELS		= 1,
	SX_CMD_READ_PIXELS_DELAYED	= 2,
	SX_CMD_READ_PIXELS		= 3,
	SX_CMD_SET_TIMER		= 4,
	SX_CMD_GET_TIMER		= 5,
	SX_CMD_RESET			= 6,
	SX_CMD_SET_CCD_PARAMS		= 7,
	SX_CMD_GET_CCD_PARAMS		= 8,
	SX_CMD_SET_STAR2K		= 9,
	SX_CMD_WRITE_SERIAL_PORT	= 10,
	SX_CMD_READ_SERIAL_PORT		= 11,
	SX_CMD_SET_SERIAL		= 12,
	SX_CMD_GET_SERIAL		= 13,
	SX_CMD_CAMERA_MODEL		= 14,
	SX_CMD_LOAD_EEPROM		= 15,
	SX_CMD_READ_PIXELS_GATED	= 18,
	SX_CMD_GET_BUILD_NUMBER		= 19,
	SX_CMD_COOLER			= 30,
	SX_CMD_COOLER_TEMPERATURE	= 31,
	SX_CMD_SHUTTER			= 32,
	SX_CMD_READ_I2CPORT		= 33
} sx_command_t;

typedef struct sx_firmware_version_s {
	uint16_t	minor_version;	// least significant byte first
	uint16_t	major_version;	// least significant byte first
} __attribute__((packed)) sx_firmware_version_t;

typedef struct sx_build_number_s {
	uint16_t	build_number;
	uint16_t	padding;
} __attribute__((packed)) sx_build_number_t;

typedef struct sx_read_pixels_s {
	uint16_t	x_offset;
	uint16_t	y_offset;
	uint16_t	width;
	uint16_t	height;
	uint8_t		x_bin;
	uint8_t		y_bin;
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

typedef struct sx_cooler_temperature_s {
	uint16_t	temperature;
	uint8_t		status;
} __attribute__((packed)) sx_cooler_temperature_t;

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _sx_h */
