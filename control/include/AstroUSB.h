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
#include <vector>
#include <iostream>
#include <tr1/memory>
#include <string.h>

#define	BROKEN_NONE			0
#define BROKEN_THE_IMAGING_SOURCE	1

namespace astro {
namespace usb {

/**
 * \brief Generic USB error
 * 
 * whenever an USB operation encounters an error, this exception is thrown.
 */
class	USBError : public std::runtime_error {
public:
	USBError(const char *error) : std::runtime_error(error) { }
};

class Context; // forward declaration for the friend class

// forward declarations for return types of descriptor query functions
class DeviceDescriptor;
typedef std::tr1::shared_ptr<DeviceDescriptor>	DeviceDescriptorPtr;

// The Configuration contains a whole hierarchy of descriptors:
//
// Configuration
//   |
//   +- Interface 0
//   |    +- InterfaceDescriptor alt setting 0
//   |    |     +- Endpoint
//   |    |     +- Endpoint
//   |    | 
//   |    +- InterfaceDescriptor alt setting 1
//   |          +- Endpoint
//   |          +- Endpoint
//   |
//   +- Interface 1
//   |    +- InterfaceDescriptor alt setting 0
//   |    |     +- Endpoint
//   |    |     +- Endpoint
//   |    | 
//   |    +- InterfaceDescriptor alt setting 1
//   |          +- Endpoint
//   |          +- Endpoint
//   |
//   +- Interface 2
//        +- InterfaceDescriptor alt setting 0
//        |     +- Endpoint
//        |     +- Endpoint
//        | 
//        +- InterfaceDescriptor alt setting 1
//              +- Endpoint
//              +- Endpoint
//
// To ensure that we can always navigate back to the top of the tree,
// each object contains a reference to the parent object.

class Configuration;
typedef std::tr1::shared_ptr<Configuration>	ConfigurationPtr;

class Device;
typedef std::tr1::shared_ptr<Device>	DevicePtr;

class Interface;
typedef std::tr1::shared_ptr<Interface>		InterfacePtr;

class InterfaceDescriptor;
typedef std::tr1::shared_ptr<InterfaceDescriptor>	InterfaceDescriptorPtr;

class EndpointDescriptor;
typedef std::tr1::shared_ptr<EndpointDescriptor>	EndpointDescriptorPtr;

class RequestBase;
typedef std::tr1::shared_ptr<RequestBase>	RequestPtr;

class Transfer;
typedef std::tr1::shared_ptr<Transfer>	TransferPtr;

/**
 * \brief Device abstraction
 *
 * The libusb device structure is a reference counted handle to devices.
 * The Device class is just a wrapper around it.
 */
class 	Device {
	libusb_device	*dev;
	libusb_device_handle	*dev_handle;

	// Device objects can only be created from a Context
	Device(struct libusb_device *dev, libusb_device_handle *dev_handle = NULL);

	// Device objects cannot be copied
	Device(const Device& other);

	void	getDescriptor(struct libusb_device_descriptor *devdesc) const;
/*
	struct libusb_device	*getDevice();
*/
	int	broken;
public:
	~Device();

	// information about the device
	uint8_t	getBusNumber() const;
	uint8_t	getDeviceAddress() const;
	int	getBroken() const;
	enum usb_speed { SPEED_UNKNOWN = 0, SPEED_LOW = 1, SPEED_FULL = 2,
		SPEED_HIGH = 3, SPEED_SUPER = 4 };
	enum usb_speed	getDeviceSpeed() const;

	// descriptor access
	DeviceDescriptorPtr	descriptor() throw(USBError);
	ConfigurationPtr	config(uint8_t index) throw(USBError);
	ConfigurationPtr	activeConfig() throw(USBError);
	ConfigurationPtr	configValue(uint8_t value) throw(USBError);
	std::string	getStringDescriptor(uint8_t index) const throw(USBError);

	// make sure the device is open
	bool	isOpen() const;
	void	open() throw(USBError);
	void	close();

	// set a configuration
	void	setConfiguration(uint8_t configuration) throw(USBError);
	int	getConfiguration() throw(USBError);

	// claim and release an interface
	void	claimInterface(uint8_t interface) throw(USBError);
	void	releaseInterface(uint8_t interface) throw(USBError);

	// request the alternate setting of an interface
	void	setInterfaceAltSetting(uint8_t interface, uint8_t altsetting)
		throw(USBError);

	// having requests processed by the device
	void	controlRequest(RequestPtr request) throw(USBError);
	void	submit(TransferPtr request) throw(USBError);

	//  some more accessors
	int	maxIsoPacketSize(uint8_t endpoint) const;

	// Context is a friend class, it acts as a factory for Devices
	friend class Context;
	friend std::ostream&	operator<<(std::ostream& out, const Device& device);
};

std::ostream&	operator<<(std::ostream& out, const Device& device);

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

	// factory functions to create Devices
	std::vector<DevicePtr>	devices() throw (USBError);
	DevicePtr	find(uint16_t vendor_id, uint16_t product_id)
		throw(USBError);
};

/*
 * request structures
 */ 
typedef struct  usb_request_header_s {
	uint8_t		bmRequestType;
	uint8_t		bRequest;
	uint16_t	wValue;
	uint16_t	wIndex;
	uint16_t	wLength;
} __attribute__((packed)) usb_request_header_t;

/**
 * \brief Request base class
 *
 * This class builds the infrastructure for sending requests on the control
 * pipe. The direction is inferred from the pointer handed in as an argument.
 * If the pointer is NULL, transfer from the host the device to the host
 * is assumed. If the pointer is not NULL, transfer from the host to the device
 * is inferred.
 *
 * The wIndex field is set to the endpoint or the interface depending on
 * whether the request was constructed for an endpoint or an interface.
 */
class RequestBase {
	// remember the direction of the request
public:
	typedef enum { host_to_device = 0, device_to_host = 0x80 }
		request_direction;
private:
	request_direction	direction;

	// remember the type of the request
public:
	typedef enum { standard_type = 0, class_specific_type = 1 << 5,
		vendor_specific_type = 2 << 5}	request_type;
private:
	request_type	type;

	// remember the recipient of the control request
public:
	typedef enum { device_recipient = 0, interface_recipient = 1,
		endpoint_recipient = 2 }	request_recipient;
private:
	request_recipient	recipient;

	uint8_t	bEndpointAddress;
	uint8_t	bInterface;

public:
	RequestBase(request_type _type, const EndpointDescriptorPtr endpoint,
		void *data = NULL);
	RequestBase(request_type type, const InterfacePtr interface,
		void *data = NULL);
	RequestBase(request_type type, void *data = NULL);
	
	virtual uint8_t	bmRequestType() const;
	virtual uint8_t	bRequest() const = 0;
	virtual uint16_t	wValue() const = 0;
	virtual uint16_t	wIndex() const;
	virtual uint16_t	wLength() const = 0;
	virtual uint8_t	*payload() const = 0;
	virtual std::string	toString() const;
};

/**
 * \brief A Request wrapper template
 *
 * Requests on the control pipe are built with this template. 
 * 
 */
template<typename T>
class Request : public RequestBase {
public:
	typedef struct request_packet_s {
		usb_request_header_t	header;
		T			payload;
	} __attribute__((packed)) request_packet_t;
private:
	request_packet_t	packet;
	void init(uint8_t bRequest, uint16_t wValue, T *payload_data) {
		packet.header.bmRequestType = RequestBase::bmRequestType();
		packet.header.bRequest = bRequest;
		packet.header.wValue = wValue;
		packet.header.wLength = sizeof(T);
		if (NULL != payload_data) {
			memcpy(&packet.payload, payload_data, sizeof(T));
		}
	}
public:	
	Request(request_type type, const EndpointDescriptorPtr endpoint,
		uint8_t bRequest, uint16_t wValue, T *payload_data = NULL)
		: RequestBase(type, endpoint, payload_data) {
		init(bRequest, wValue, payload_data);
		packet.header.wIndex = RequestBase::wIndex();
	}

	Request(request_type type, const InterfacePtr& interface,
		uint8_t bRequest, uint16_t wValue, T *payload_data = NULL)
		: RequestBase(type, interface, payload_data) {
		init(bRequest, wValue, payload_data);
		packet.header.wIndex = RequestBase::wIndex();
	}

	Request(request_type type, uint16_t wIndex,
		uint8_t bRequest, uint16_t wValue, T *payload_data = NULL)
		: RequestBase(type, payload_data) {
		init(bRequest, wValue, payload_data);
		packet.header.wIndex = wIndex;
	}

	// accessors to the public data
	virtual uint8_t	*payload() const {
		return (uint8_t *)&packet.payload;
	}

	virtual T	*data() {
		return &packet.payload;
	}

	virtual uint8_t	bmRequestType() const {
		return packet.header.bmRequestType;
	}

	virtual uint8_t bRequest() const {
		return packet.header.bRequest;
	}

	virtual uint16_t	wValue() const {
		return packet.header.wValue;
	}

	virtual uint16_t	wIndex() const {
		return packet.header.wIndex;
	}

	virtual uint16_t	wLength() const {
		return sizeof(T);
	}
};

/**
 * \brief empty requests
 *
 * C does not allow empty structures, so the Request Template does not work
 * for empty 
 */
class EmptyRequest : public RequestBase {
	usb_request_header_t	header;
	void	init(uint8_t bRequest, uint16_t wValue);
public:
	EmptyRequest(request_type type, const EndpointDescriptorPtr endpoint,
		uint8_t bRequest, uint16_t wValue);
	EmptyRequest(request_type type, const InterfacePtr interface,
		uint8_t bRequest, uint16_t wValue);
	EmptyRequest(request_type type, uint16_t wIndex,
		uint8_t bRequest, uint16_t wValue);

	virtual uint8_t	bmRequestType() const;
	virtual uint8_t bRequest() const;
	virtual uint16_t	wValue() const;
	virtual uint16_t	wIndex() const;
	virtual uint16_t	wLength() const;
	virtual uint8_t	*payload() const;
};

/**
 * \brief Transfer base class
 *
 * This class holds the common stuff, derivced classes implement the
 * transfer specific stuff like bulk or isochronous transfers.
 */
class Transfer {
protected:
	uint8_t	endpoint;
	int	length;
	unsigned char	*data;
	libusb_transfer	*transfer;
	bool	freedata;
	int	timeout;
private:
	virtual void	submit(libusb_device_handle *devhandle) throw(USBError) = 0;
public:
	virtual void	callback() = 0;
	Transfer(uint8_t endpoint, int length, unsigned char *data = NULL);
	virtual ~Transfer();
	int	getTimeout() const;
	void	setTimeout(int timeout);

	friend class Device;
};

class BulkTransfer : public Transfer {
public:
	virtual void	callback();
private:
	virtual void	submit(libusb_device_handle *devhandle) throw(USBError);
public:
	BulkTransfer(uint8_t endpoint, int length, unsigned char *data = NULL);
	virtual ~BulkTransfer();
};

class IsoTransfer : public Transfer {
public:
	virtual void	callback();
private:
	virtual void	submit(libusb_device_handle *devhandle) throw(USBError);
public:
	IsoTransfer(uint8_t endpoint, int length, unsigned char *data = NULL);
	virtual ~IsoTransfer();
};

/**
 * \brief Device Descriptor abstraction
 *
 * This class is not simply a copy of the libusb_device_descriptor, but
 * it expands the strings referenced in the descriptor
 */
class	DeviceDescriptor {
	Device&	dev;
	struct libusb_device_descriptor	d;
	std::string	manufacturer;
	std::string	product;
	std::string	serialnumber;
	DeviceDescriptor(Device& device,
		libusb_device_descriptor *dev_descriptor);
	DeviceDescriptor(const DeviceDescriptor& other);
public:
	// constructors
	~DeviceDescriptor();

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
	Device&	dev;
	std::string	extra_descriptors;
	Descriptor(const Descriptor& other);
public:
	Descriptor(Device& device, const void *extra, int extra_length);
	Descriptor(Device& device, const std::string& extra);
	Device&	device();
	const Device&	device() const;
	const std::string&	extra() const;
};

typedef std::tr1::shared_ptr<Descriptor>	DescriptorPtr;

/**
 * \brief USB Endpoint Descriptor
 *
 * Each alternate setting for an interface contains some endpoint descriptors.
 */
class EndpointDescriptor : public Descriptor {
	struct libusb_endpoint_descriptor	*epd;
	void	copy(const struct libusb_endpoint_descriptor	*epd);
	InterfaceDescriptor&	interfacedescriptor;
	EndpointDescriptor(const EndpointDescriptor& other);
public:
	// constructors
	EndpointDescriptor(Device& device, InterfaceDescriptor& interface,
		const libusb_endpoint_descriptor *epd);
	~EndpointDescriptor();

	// accessors
	uint8_t		bEndpointAddress() const;
	uint8_t		bmAttributes() const;
	uint16_t	wMaxPacketSize() const;
	uint8_t		bInterval() const;
	uint8_t		bRefresh() const;
	uint8_t		bSynchAddress() const;

	// accessor to the interface
	InterfaceDescriptor&	interface();

	// text output
	std::string	toString() const;
};

typedef std::tr1::shared_ptr<EndpointDescriptor>	EndpointDescriptorPtr;

std::ostream&	operator<<(std::ostream& out, const EndpointDescriptor& epd);

/**
 * \brief USB Interface Descriptor
 */
class InterfaceDescriptor : public Descriptor {
	struct libusb_interface_descriptor	*ifdp;
	void	copy(const struct libusb_interface_descriptor *ifpd);
	std::string	interfacename;
	Interface&	interface;
	// an interface descriptor has exactly one endpoint descriptor,
	// which is used for bandwith negotiation
	std::vector<EndpointDescriptorPtr>	endpointlist;
	void	getEndpoints();
	InterfaceDescriptor(const InterfaceDescriptor& other);
public:
	InterfaceDescriptor(Device& device, Interface& interface,
		const struct libusb_interface_descriptor *ifdp);

	uint8_t	bInterfaceNumber() const;
	uint8_t	bAlternateSetting() const;
	uint8_t	bInterfaceClass() const;
	uint8_t	bInterfaceSubClass() const;
	uint8_t	bInterfaceProtocol() const;
	const std::string&	iInterface() const;

	// use this interface descriptors alt setting
	void	altSetting() throw(USBError);

	// access the various endpoint descriptors (int the cases we are
	// interested in, there is only a single endpoint descriptor)
	int	numEndpoints() const;
	EndpointDescriptorPtr	operator[](size_t index) const;

	// display
	std::string	toString() const;
};

typedef	std::tr1::shared_ptr<InterfaceDescriptor>	InterfaceDescriptorPtr;

std::ostream&	operator<<(std::ostream& out, const InterfaceDescriptor& ifd);

/**
 * \brief Interface abstraction, contains all the alternate settings
 */
class Interface {
	Device&	dev;
	std::vector<InterfaceDescriptorPtr>	altsettingvector;
	int	interface;
	Configuration&	configuration;
	Interface(const Interface& other);
public:
	Interface(Device& device, Configuration& configuration,
		const libusb_interface *li, int interface);
	size_t	numAltsettings() const;
	int	interfaceNumber() const;
	const InterfaceDescriptorPtr&	operator[](size_t index) const;
	InterfaceDescriptorPtr&	operator[](size_t index);
	void	claim() throw(USBError);
	void	release() throw(USBError);
	std::string	toString() const;
};

typedef std::tr1::shared_ptr<Interface>	InterfacePtr;

std::ostream&	operator<<(std::ostream& out, const Interface& interface);

/**
 * \brief USB Configuration Descriptor class
 */
class	Configuration : public Descriptor {
	libusb_config_descriptor	*config;
	std::string	configuration;
	void	copy(const libusb_config_descriptor *config);
	std::vector<InterfacePtr>	interfacelist;
	void	getInterfaces();
	Configuration(const Configuration& other);
public:
	Configuration(Device& device,
		const struct libusb_config_descriptor *config);
	~Configuration();
	uint8_t	bConfigurationValue() const;
	uint8_t	bNumInterfaces() const;
	uint8_t	bmAttributes() const;
	uint8_t	MaxPower() const;

	// configure this configuration
	void	configure() throw(USBError);

	// access to the interfaces of this configuration
	const std::vector<InterfacePtr>&	interfaces() const;
	const InterfacePtr&	operator[](size_t index) const;
	InterfacePtr&	operator[](size_t index);
};

std::ostream&	operator<<(std::ostream& out, const Configuration& config);

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
	USBDescriptor(const USBDescriptor& other);
protected:
	Device&	device;
	uint8_t	*data;
	uint8_t	uint8At(int offset) const;
	int8_t	int8At(int offset) const;
	uint16_t	uint16At(int offset) const;
	int16_t	int16At(int offset) const;
	uint32_t	uint32At(int offset) const;
	int32_t	int32At(int offset) const;
	uint32_t	bitmapAt(int offset, int size) const;
public:
	USBDescriptor(Device& device, const void *data, int length);
	virtual	~USBDescriptor();
	virtual	std::string	toString() const;
	uint8_t	bLength() const;
	uint8_t	bDescriptorType() const;
	virtual int	descriptorLength() const;
};

typedef std::tr1::shared_ptr<USBDescriptor>	USBDescriptorPtr;

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
	UnknownDescriptorError(uint8_t length, uint8_t descriptortype,
		uint8_t descriptorsubtype);
};

/**
 * \brief USB Descriptor factory for descriptors that do not have a structure
 *
 */

std::ostream&	operator<<(std::ostream& out,
	const USBDescriptorPtr& descriptorptr);

class DescriptorFactory {
protected:
	Device&	device;
	uint8_t	blength(const void *data) throw(std::length_error);
	uint8_t	bdescriptortype(const void *data);
	DescriptorFactory(const DescriptorFactory& other);
public:
	DescriptorFactory(Device& device);

	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);

	USBDescriptorPtr	descriptor(const std::string& data)
		throw(std::length_error, UnknownDescriptorError);

	virtual std::vector<USBDescriptorPtr>	descriptors(const void *data,
		int length)
		throw(std::length_error, UnknownDescriptorError);

	std::vector<USBDescriptorPtr>	descriptors(const std::string& data)
		throw(std::length_error, UnknownDescriptorError);
};

std::ostream&	operator<<(std::ostream& out,
			const std::vector<USBDescriptorPtr>& list);

/**
 * \brief Inteface association descriptor
 *
 * An interface association descriptor is a standard USB descriptor describing
 * a collection of interfaces that are used for a single function. In the case
 * of a USB video class function, the interface association descriptor 
 * describes the one video control interface and the one or more video
 * streaming interfaces of the video function.
 */
class InterfaceAssociationDescriptor : public USBDescriptor {
	std::string	function;
	InterfaceAssociationDescriptor(const InterfaceAssociationDescriptor& iad);
public:
	InterfaceAssociationDescriptor(Device& device,
		const void *data, int length);
	uint8_t	bFirstInterface() const;
	uint8_t	bInterfaceCount() const;
	uint8_t	bFunctionClass() const;
	uint8_t	bFunctionSubClass() const;
	uint8_t	bFunctionProtocol() const;
	const std::string&	iFunction() const;
	virtual std::string	toString() const;
	bool	isVideoInterfaceCollection() const;
};

bool	isInterfaceAssociationDescriptor(const USBDescriptorPtr& ptr);

InterfaceAssociationDescriptor	*interfaceAssociationDescriptor(
	const USBDescriptorPtr& ptr);

} // namespace usb
} // namespace astro

#endif /* _AstroUSB_h */
