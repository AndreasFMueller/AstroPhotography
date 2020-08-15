/*
 * SxUtils.h -- utilities for 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxUtils_h
#define _SxUtils_h

#include <stdexcept>
#include <AstroUSB.h>
#include <DeviceNameUSB.h>
#include "sx.h"

namespace astro {
namespace camera {
namespace sx {

#define SX_MODEL_M26C   0x005a
#define SX_MODEL_56     0x0021
#define SX_MODEL_46     0x0022
#define SX_PRODUCT_M26C 0x0326

extern std::string	command_name(sx_command_t command);

class SxError : public std::runtime_error {
public:
	SxError(const char *cause);
};

/*
 * the last constant is not contained in the official documentation,
 * it is a meaning of that by conjectured in an email from Terry Platt, 
 * <tplatt@starlight.win-uk.net>, may 4, 2013
 */
typedef struct sx_model_s {
	unsigned short  product;
	unsigned short  model;
	const char	*name;
	const char	*friendlyname;
	bool		hascooler;
} sx_model_t;

/**
 * \brief A class 
 */
class SxName : public DeviceName {
	static const size_t	number_sx_models;
	static sx_model_t	models[];
	unsigned short	_product;
	unsigned short	_model;
	bool	_hascooler;
public:

	SxName(DeviceName::device_type type, usb::DevicePtr deviceptr);

	unsigned short	product() const { return _product; }
	unsigned short	model() const { return _model; }
	bool	hascooler() const { return _hascooler; }

	static std::string	userFriendlyName(unsigned short product,
					unsigned short model);
	static std::string	deviceName(unsigned short product,
					unsigned short model);
	static bool	hasCooler(unsigned short product,
				unsigned short model);
	static std::string	deviceName(usb::DevicePtr deviceptr);

	DeviceName	cameraname() const;
	DeviceName	coolername() const;
	DeviceName	ccdname() const;
	DeviceName	guideportname() const;
	static DeviceName	cameraname(const DeviceName& other);
	static DeviceName	coolername(const DeviceName& other);
	static DeviceName	ccdname(const DeviceName& other);
	static DeviceName	guideportname(const DeviceName& other);
};

extern std::string     wchar2string(const wchar_t *w);

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxUtils_h */
