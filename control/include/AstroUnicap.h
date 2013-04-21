/*
 * AstroUnicap.h -- C++ adapter for the Unicap library
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdexcept>

#ifdef HAVE_UNICAP_H

extern "C" {
#include <unicap.h>
}

namespace astro {
namespace unicap {

/**
 * \brief Exception class for Unicap errors.
 *
 * This exception is thrown for runtime errors within the Unicap library
 */
class UnicapError : public std::runtime_error {
public:
	UnicapError(const char *cause);
	UnicapError(unicap_status_t status, const char *cause);
};

/**
 * \brief Class representing a Unicap rectangle size
 */
class UnicapFormat;
class UnicapRectangle {
	unicap_rect_t	rect;
	UnicapRectangle(unicap_rect_t *rect);
public:
	UnicapRectangle();
	int	x();
	int	y();
	int	width();
	int	height();
	friend class UnicapFormat;
};

/**
 * \brief Class representing a Unicap format.
 */
class UnicapDevice;
class UnicapFormat {
	unicap_format_t	format;
	UnicapFormat(unicap_format_t *format);
public:
	UnicapFormat(const UnicapFormat& other);
	~UnicapFormat();
	std::string	identifier();
	int	numSizes();
	friend class UnicapDevice;
	UnicapRectangle	get(int i);
};


/**
 * \brief Class representing a Unicap device.
 */
class Unicap;
class UnicapDevice {
	unicap_handle_t	handle;
	bool	isopen;
	UnicapDevice(unicap_device_t *device);
public:
	UnicapDevice(const UnicapDevice& other);
	~UnicapDevice();
	friend class Unicap;

	std::string	identifier();
	std::string	model_name();
	std::string	vendor_name();
	unsigned long long	model_id();
	unsigned int	vendor_id();

	int	numFormats();
	UnicapFormat	getFormat(int index);
};

/**
 * \brief Factory class toget Unicap devices
 *
 * This class allows enumerating Unicap devices
 */
class Unicap {
public:
	Unicap();
	int	numDevices();
	UnicapDevice	get(int index);
};


} // namespace unicap
} // namespace astro

#endif /* HAVE_UNICAP_H */
