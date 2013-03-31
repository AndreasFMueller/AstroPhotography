/*
 * AstroUSB.h -- base classes to handle devices that use a USB connection
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroUSB_h
#define _AstroUSB_h

#include <libusb-1.0/libusb.h>
#include <stdexcept>
#include <list>
#include <iostream>
#include <tr1/memory>

namespace astro {
namespace usb {

class	USBError : public std::runtime_error {
public:
	USBError(const char *error) : std::runtime_error(error) { }
};

class	Device;
class	DeviceHandle;

/**
 * \brief Class to encapsulate libusb_context
 *
 * In libusb, all USB operations have to take place within an USB context.
 * So an instance of this class has to be created before any USB operations
 * are attempted, and no USB operations are allowed after this instance
 * has gone out of scope.
 */
class	Context {
	libusb_context	*context;
	Context(const Context& other);
public:
	Context() throw(USBError);
	~Context();
	void	setDebugLevel(int level) throw(std::range_error);
	std::list<Device>	list() throw (USBError);
	DeviceHandle	*open(uint16_t vendor_id, uint16_t product_id)
		throw(USBError);
};

class DeviceDescriptor;
class ConfigDescriptor;

/**
 * \brief Device abstraction
 *
 * The libusb device structure is a reference counted handle to devices.
 * The Device class is just a wrapper around it.
 */
class 	Device {
	struct libusb_device	*dev;
	Device(struct libusb_device *dev);
	void	getDescriptor(struct libusb_device_descriptor *devdesc) const;
	struct libusb_device	*getDevice();
public:
	Device(const Device& other);
	~Device();
	Device&	operator=(const Device& other);
	uint8_t	getBusNumber() const;
	uint8_t	getDeviceAddress() const;
	int	getDeviceSpeed() const;
	DeviceDescriptor	*descriptor() const throw(USBError);
	ConfigDescriptor	*config(uint8_t index) const throw(USBError);
	ConfigDescriptor	*activeConfig() const throw(USBError);
	ConfigDescriptor	*configValue(uint8_t value) const throw(USBError);
	DeviceHandle	*open() throw(USBError);
	friend class Context;
	friend class DeviceDescriptor;
	friend std::ostream&	operator<<(std::ostream& out, const Device& device);
};

std::ostream&	operator<<(std::ostream& out, const Device& device);

/**
 * \brief USB Device Handle
 */

class	DeviceHandle {
	libusb_device_handle	*dev_handle;
	Device	dev;
	DeviceHandle(const Device& _device, libusb_device_handle *handle);
public:
	~DeviceHandle();
	Device	device();
	int	getConfiguration() throw (USBError);
	void	setConfiguration(int configuration) throw (USBError);
	void	claimInterface(int interface) throw (USBError);
	void	releaseInterface(int interface) throw (USBError);
	void	reset() throw (USBError);
	std::string	getStringDescriptor(uint8_t index) const;
	friend class Context;
	friend class Device;
};

/**
 * \brief Device Descriptor abstraction
 *
 * This class is not simply a copy of the libusb_device_descriptor, but
 * it expands the strings referenced in the descriptor
 */
class	DeviceDescriptor {
	Device	dev;
	struct libusb_device_descriptor	*d;
	std::string	manufacturer;
	std::string	product;
	std::string	serialnumber;
	DeviceDescriptor(const Device& device);
public:
	// constructors
	~DeviceDescriptor();
	DeviceDescriptor(const DeviceDescriptor& other);
	DeviceDescriptor&	operator=(const DeviceDescriptor& other);

	uint16_t	bcdUSB() const;
	uint8_t		bDeviceClass() const;
	uint8_t		bDeviceSubClass() const;
	uint8_t		bDeviceProtocol() const;
	uint8_t		bMaxPacketSize0() const;
	uint16_t	idVendor() const;
	uint16_t	idProduct() const;
	uint16_t	bcdDevice() const;
	const std::string&	iManufacturer() const;
	const std::string&	iProduct() const;
	const std::string&	iSerialNumber() const;
	uint8_t	bNumConfigurations() const;
	friend class Device;
};

std::ostream&	operator<<(std::ostream& out, const DeviceDescriptor& devdesc);

/**
 * \brief common Descriptor base class
 */
class Descriptor {
protected:
	Device	dev;
	std::string	extra_descriptors;
public:
	Descriptor(const Device& device, const void *extra, int extra_length);
	Descriptor(Device& device, const std::string& extra);
	Device&	device();
	const Device&	device() const;
	const std::string&	extra() const;
};

typedef std::tr1::shared_ptr<Descriptor>	DescriptorPtr;

/**
 * \brief USB Endpoint Descriptor
 */
class EndpointDescriptor : public Descriptor {
	struct libusb_endpoint_descriptor	*epd;
	void	copy(const struct libusb_endpoint_descriptor	*epd);
public:
	// constructors
	EndpointDescriptor(const Device& device,
		const libusb_endpoint_descriptor *epd);
	~EndpointDescriptor();
	EndpointDescriptor(const EndpointDescriptor& other);
	EndpointDescriptor&	operator=(const EndpointDescriptor& other);

	// accessors
	uint8_t		bEndpointAddress() const;
	uint8_t		bmAttributes() const;
	uint16_t	wMaxPacketSize() const;
	uint8_t		bInterval() const;
	uint8_t		bRefresh() const;
	uint8_t		bSynchAddress() const;
};

std::ostream&	operator<<(std::ostream& out, const EndpointDescriptor& epd);

/**
 * \brief USB Interface Descriptor
 */
class InterfaceDescriptor : public Descriptor {
	struct libusb_interface_descriptor	*ifdp;
	void	copy(const struct libusb_interface_descriptor *ifpd);
	std::list<EndpointDescriptor>	endpoints;
	void	getEndpoints();
	std::string	interface;
public:
	InterfaceDescriptor(const Device& device,
		const struct libusb_interface_descriptor *ifdp);
	InterfaceDescriptor(const InterfaceDescriptor& other);
	InterfaceDescriptor&	operator=(const InterfaceDescriptor& other);

	uint8_t	bInterfaceNumber() const;
	uint8_t	bAlternateSetting() const;
	uint8_t	bInterfaceClass() const;
	uint8_t	bInterfaceSubClass() const;
	uint8_t	bInterfaceProtocol() const;
	const std::string&	iInterface() const;
	const std::list<EndpointDescriptor>&	endpoint() const;
};

std::ostream&	operator<<(std::ostream& out, const InterfaceDescriptor& ifd);

/**
 * \brief Interface abstraction, contains all the alternate settings
 */
class Interface {
	Device	dev;
	std::list<InterfaceDescriptor>	altsetting;
public:
	Interface(const Device& device, const libusb_interface *li);
	const std::list<InterfaceDescriptor>&	altsettings() const;
};

std::ostream&	operator<<(std::ostream& out, const Interface& interface);

/**
 * \brief USB Configuration Descriptor class
 */
class	ConfigDescriptor : public Descriptor {
	libusb_config_descriptor	*config;
	std::string	configuration;
	void	copy(const libusb_config_descriptor *config);
	std::list<Interface>	interfaces;
	void	getInterfaces();
public:
	ConfigDescriptor(const Device& device,
		const struct libusb_config_descriptor *config);
	ConfigDescriptor(const ConfigDescriptor& other);
	ConfigDescriptor&	operator=(const ConfigDescriptor& other);
	~ConfigDescriptor();
	uint8_t	bConfigurationValue() const;
	uint8_t	bNumInterfaces() const;
	uint8_t	bmAttributes() const;
	uint8_t	MaxPower() const;
	const std::list<Interface>&	interface() const;
};

std::ostream&	operator<<(std::ostream& out, const ConfigDescriptor& config);

/**
 * \brief Generic USB descriptor
 *
 * Not all descriptors have associated structures in libusb, so we need
 * some method to extend the descriptor mechanism. All descriptors to
 * have a length field, and 
 */
class USBDescriptor {
	uint8_t	blength;
	uint8_t	bdescriptortype;
protected:
	Device	device;
	uint8_t	*data;
public:
	USBDescriptor(const Device& device, const void *data, int length);
	USBDescriptor(const USBDescriptor& other);
	USBDescriptor&	operator=(const USBDescriptor& other);
	virtual	~USBDescriptor();
	virtual	std::string	toString() const;
	uint8_t	bLength() const;
	uint8_t	bDescriptorType() const;
};

std::ostream&	operator<<(std::ostream& out, const USBDescriptor& descriptor);

/**
 * \brief UnknownDescriptorError
 *
 * For descriptors that don't have a libusb equivalent, this error is thrown
 * by the respective factories when construction of a descriptor class
 * is attempted.
 */
class UnknownDescriptorError : public std::runtime_error {
public:
	UnknownDescriptorError(const char *cause) : std::runtime_error(cause) {}
	UnknownDescriptorError(uint8_t length, uint8_t descriptortype);
};

/**
 * \brief USB Descriptor factory for descriptors that do not have a structure
 *
 */
typedef	std::tr1::shared_ptr<USBDescriptor>	USBDescriptorPtr;

std::ostream&	operator<<(std::ostream& out,
	const USBDescriptorPtr& descriptorptr);

class DescriptorFactory {
protected:
	Device	device;
public:
	DescriptorFactory(const Device& device);
	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);
	virtual std::list<USBDescriptorPtr>	descriptors(const void *data,
		int length) throw(std::length_error, UnknownDescriptorError);
};

std::ostream&	operator<<(std::ostream& out,
			const std::list<USBDescriptorPtr>& list);

/**
 * \brief Inteface association descriptor
 */
class InterfaceAssociationDescriptor : public USBDescriptor {
	std::string	function;
public:
	InterfaceAssociationDescriptor(const Device& device,
		const void *data, int length);
	InterfaceAssociationDescriptor(const InterfaceAssociationDescriptor& other);
	InterfaceAssociationDescriptor&	operator=(const InterfaceAssociationDescriptor& other);
	uint8_t	bFirstInterface() const;
	uint8_t	bInterfaceCount() const;
	uint8_t	bFunctionClass() const;
	uint8_t	bFunctionSubClass() const;
	uint8_t	bFunctionProtocol() const;
	const std::string&	iFunction() const;
	virtual std::string	toString() const;
};

} // namespace usb
} // namespace astro

#endif /* _AstroUSB_h */
