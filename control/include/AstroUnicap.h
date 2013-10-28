/*
 * AstroUnicap.h -- C++ adapter for the Unicap library
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <AstroUVC.h>

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
 * \brief Base class for all properties
 *
 * The base class of the properties just gives access to the common
 * attributes identifying the property.
 */
class UnicapProperty {
protected:
	unicap_property_t	property;
	UnicapProperty(unicap_property_t *property);
public:
	~UnicapProperty();
	std::string	identifier() const;
	std::string	category() const;
	std::string	unit() const;
	virtual std::string	toString() const;
};

typedef std::shared_ptr<UnicapProperty>	UnicapPropertyPtr;

std::ostream&	operator<<(std::ostream& out, const UnicapProperty& prop);

/**
 * \brief Base class for number valued properties.
 *
 * The range and valuelist properties have double values, this common
 * class gives access to the value.
 */
class UnicapPropertyDouble : public UnicapProperty {
protected:
	UnicapPropertyDouble(unicap_property_t *property);
public:
	virtual	~UnicapPropertyDouble();
	double	value() const;
	virtual std::string	toString() const;
};

/**
 * \brief Range property.
 *
 * Range properties can take any value in a range.
 */
class UnicapPropertyRange : public UnicapPropertyDouble {
public:
	UnicapPropertyRange(unicap_property_t *property);
	virtual	~UnicapPropertyRange();
	double	getMin() const;
	double	getMax() const;
	virtual std::string	toString() const;
};

/**
 * \brief Value list property.
 *
 * Valuelist properties take values from a list of valid values.
 */
class UnicapPropertyValuelist : public UnicapPropertyDouble {
public:
	UnicapPropertyValuelist(unicap_property_t *property);
	virtual	~UnicapPropertyValuelist();
	std::vector<double>	values() const;
	virtual std::string	toString() const;
};

/**
 * \brief Menu property.
 *
 * Menu properties take discrete values from a list, the values
 * are identified by strings.
 */
class UnicapPropertyMenu : public UnicapProperty {
public:
	UnicapPropertyMenu(unicap_property_t *property);
	virtual	~UnicapPropertyMenu();
	std::vector<std::string>	items() const;
	std::string	item() const;
	virtual std::string	toString() const;
};

/**
 * \brief Data property.
 *
 * Data properties have just a datablock as data.
 */
class UnicapPropertyData : public UnicapProperty {
public:
	UnicapPropertyData(unicap_property_t *property);
	virtual	~UnicapPropertyData();
	virtual std::string	toString() const;
};

/**
 * \brief Flag properties.
 *
 * Flags properties take flag values.
 */
class UnicapPropertyFlags : public UnicapProperty {
public:
	UnicapPropertyFlags(unicap_property_t *property);
	virtual	~UnicapPropertyFlags();
	virtual std::string	toString() const;
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
	int	x() const;
	int	y() const;
	int	width() const;
	int	height() const;
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
	std::string	identifier() const;
	int	numSizes() const;
	friend class UnicapDevice;
	UnicapRectangle	get(int i);
	void	setBufferType(unicap_buffer_type_t type);
	std::string	toString() const;
};

std::ostream&	operator<<(std::ostream& out, const UnicapFormat& format);

/**
 * \brief Class representing a Unicap device.
 */
class Unicap;
class UnicapDevice {
	int	nformats;
	int	nproperties;
	unicap_handle_t	handle;
	bool	isopen;
	UnicapDevice(unicap_device_t *device);
	int	width;
	int	height;
	std::vector<astro::usb::FramePtr>	frames;
public:
	UnicapDevice(const UnicapDevice& other);
	~UnicapDevice();
	friend class Unicap;

	std::string	identifier() const;
	std::string	model_name() const;
	std::string	vendor_name() const;
	unsigned long long	model_id() const;
	unsigned int	vendor_id() const;

	int	numFormats() const;
	UnicapFormat	getFormat(int index);
	void	setFormat(UnicapFormat& format);
	int	numProperties() const;
	UnicapPropertyPtr	getProperty(int index);
	std::vector<astro::usb::FramePtr>	getFrames(size_t count);
	std::string	toString() const;
	void	callback(unicap_event_t event, unicap_data_buffer_t *buffer);
};

std::ostream&	operator<<(std::ostream& out, const UnicapDevice& device);

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
