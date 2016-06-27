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
#include <queue>
#include <iostream>
#include <memory>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>

#define	CC_VIDEO			0x0e

#define SC_UNDEFINED			0x00
#define SC_VIDECONTROL			0x01
#define SC_VIDEOSTREAMING		0x02
#define SC_VIDEO_INTERFACE_COLLECTION	0x03

#define	PC_PROTOCOL_UNDEFINED		0x00

/* constants identifying broken cameras */
#define	BROKEN_NONE			0
#define BROKEN_THE_IMAGING_SOURCE	1
#define BROKEN_ZWO			0

#define	VENDOR_THE_IMAGING_SOURCE	0x199e
#define VENDOR_ZWO			0x03c3

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

/**
 * \brief Holder class for USB context
 */
class ContextHolder {
	libusb_context	*_context;
public:
	ContextHolder();
	~ContextHolder();
	libusb_context	*context() { return _context; }
};
typedef std::shared_ptr<ContextHolder>	ContextHolderPtr;

class Context; // forward declaration for the context class
typedef std::shared_ptr<Context>	ContextPtr;

// forward declarations for return types of descriptor query functions
class DeviceDescriptor;
typedef std::shared_ptr<DeviceDescriptor>	DeviceDescriptorPtr;

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
typedef std::shared_ptr<Configuration>	ConfigurationPtr;

class Device;
typedef std::shared_ptr<Device>	DevicePtr;

class Interface;
typedef std::shared_ptr<Interface>		InterfacePtr;

class InterfaceDescriptor;
typedef std::shared_ptr<InterfaceDescriptor>	InterfaceDescriptorPtr;

class EndpointDescriptor;
typedef std::shared_ptr<EndpointDescriptor>	EndpointDescriptorPtr;

class USBDescriptor;
typedef std::shared_ptr<USBDescriptor>	USBDescriptorPtr;

class RequestBase;
typedef std::shared_ptr<RequestBase>	RequestPtr;

class Transfer;
typedef std::shared_ptr<Transfer>	TransferPtr;

/**
 * \brief Device abstraction
 *
 * The libusb device structure is a reference counted handle to devices.
 * The Device class is just a wrapper around it.
 */
class 	Device {
	ContextHolderPtr	context;
	libusb_device	*dev;
	libusb_device_handle	*dev_handle;

	// Device objects can only be created from a Context
	Device(ContextHolderPtr context, libusb_device *dev,
		libusb_device_handle *dev_handle = NULL);

	// Device objects cannot be copied
	Device(const Device& other);

	void	getDescriptor(struct libusb_device_descriptor *devdesc) const;
	int	broken;
public:
	~Device();

	// information about the device
	uint8_t	getBusNumber() const;
	uint8_t	getDeviceAddress() const;
	std::string	getDeviceName() const;
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
	ContextHolderPtr	getContext() const;

	// find all the interface association descriptors
	std::list<USBDescriptorPtr>	interfaceAssociationDescriptors(bool videoonly)
		throw (USBError);
	bool	isVideoDevice();

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
	void	controlRequest(RequestBase *request) throw(USBError);
	void	submit(Transfer *request) throw(USBError);

	//  some more accessors
	int	maxIsoPacketSize(uint8_t endpoint) const;

	// kernel drivers
	bool	kernelDriverActive(uint8_t interface) const throw(USBError);
	void	detachKernelDriver(uint8_t interface) const throw(USBError);
	void	attachKernelDriver(uint8_t interface) const throw(USBError);

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
	ContextHolderPtr	context;
	Context(const Context& other);
public:
	Context() throw(USBError);
	~Context();
	void	setDebugLevel(int level) throw(std::range_error);

	// factory functions to create Devices
	std::vector<DevicePtr>	devices() throw (USBError);
	DevicePtr	find(uint16_t vendor_id, uint16_t product_id)
		throw(USBError);
	libusb_context	*getLibusbContext() const;
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
	int	timeout;
public:
	int	getTimeout() const { return timeout; }
	void	setTimeout(int _timeout) { timeout = _timeout; }
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
protected:
	void	init_data(const void *data);
public:
	bool	accept_short_response;
	RequestBase(request_type _type, const EndpointDescriptorPtr endpoint,
		const void *data = NULL);
	RequestBase(request_type type, const InterfacePtr interface,
		const void *data = NULL);
	RequestBase(request_type type, request_recipient recipient,
		const void *data = NULL);
	
	virtual uint8_t	bmRequestType() const;
	virtual uint8_t	bRequest() const = 0;
	virtual uint16_t	wValue() const = 0;
	virtual uint16_t	wIndex() const;
	virtual uint16_t	wLength() const = 0;
	virtual const usb_request_header_t&	getHeader() const = 0;
	virtual const void	*getPacket() const = 0;
private:
	virtual void	setwLength(int length) = 0;
public:
	virtual uint8_t	*payload() const = 0;
	virtual std::string	toString() const;
	virtual std::string	payloadHex() const;
};

/**
 * \brief Request template.
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
	/**
	 * \brief Initialize the request.
	 */
	void init(uint8_t bRequest, uint16_t wValue, const T *payload_data) {
		packet.header.bmRequestType = RequestBase::bmRequestType();
		packet.header.bRequest = bRequest;
		packet.header.wValue = wValue;
		packet.header.wLength = sizeof(T);
		init_data(payload_data);
	}
public:	
	/**
	 * \brief Construct a request to an endpoint.
	 */
	Request(request_type type, const EndpointDescriptorPtr endpoint,
		uint8_t bRequest, uint16_t wValue, const T *payload_data = NULL)
		: RequestBase(type, endpoint, payload_data) {
		init(bRequest, wValue, payload_data);
		packet.header.wIndex = RequestBase::wIndex();
	}

	/**
	 * \brief Construct a request to an interface.
	 *
	 * Fill the structure of a request that is addressed to an
	 * interface. In most cases, this is a video streaming interface.
	 * \param type		request type
	 * \param interface	
	 * \param bRequest
	 * \param wValue
	 * \param payload_data	pointer to the payload data.
	 *			If the payload_data is NULL, then this is a
	 *			request that retrieves data from the device.
	 */
	Request(request_type type, const InterfacePtr& interface,
		uint8_t bRequest, uint16_t wValue, const T *payload_data = NULL)
		: RequestBase(type, interface, payload_data) {
		init(bRequest, wValue, payload_data);
		packet.header.wIndex = RequestBase::wIndex();
	}

	/**
	 * \brief Construct a raw request.
	 *
	 * Fill the request structure directly.
	 * \param type		request type
	 * \param recipient	request recipient
	 * \param wIndex	
	 * \param bRequest	request number
	 * \param wValue	
	 * \param payload_data	pointer to the data structure to send or
	 * 			retrieve
	 */
	Request(request_type type, request_recipient recipient, uint16_t wIndex,
		uint8_t bRequest, uint16_t wValue, const T *payload_data = NULL)
		: RequestBase(type, recipient, payload_data) {
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

	virtual const usb_request_header_t&	getHeader() const {
		return packet.header;
	}

	virtual const void	*getPacket() const {
		return &packet;
	}
private:
	virtual void	setwLength(int length) {
		packet.header.wLength = length;
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
	EmptyRequest(request_type type, request_recipient recipient,
		uint16_t wIndex,
		uint8_t bRequest, uint16_t wValue);

	virtual uint8_t	bmRequestType() const;
	virtual uint8_t bRequest() const;
	virtual uint16_t	wValue() const;
	virtual uint16_t	wIndex() const;
	virtual uint16_t	wLength() const;
	virtual const usb_request_header_t&	getHeader() const;
private:
	virtual void	setwLength(int length);
public:
	virtual uint8_t	*payload() const;
	virtual const void	*getPacket() const;
};

/**
 * \brief Transfer base class
 *
 * This class holds the common stuff, derivced classes implement the
 * transfer specific stuff like bulk or isochronous transfers.
 */
class Transfer {
protected:
	EndpointDescriptorPtr	endpoint;
	int	timeout;
	bool	complete;
	libusb_context	*getContext();
private:
	virtual void	submit(libusb_device_handle *devhandle)
		throw(USBError) = 0;
public:
	virtual void	callback(libusb_transfer *transfer) = 0;
	Transfer(EndpointDescriptorPtr endpoint);
	virtual ~Transfer();
	int	getTimeout() const;
	void	setTimeout(int timeout);

	bool	isComplete() const;

	friend class Device;
};

/**
 * \brief Transfer class for bulk transfers.
 */
class BulkTransfer : public Transfer {
protected:
	int	length;
	unsigned char	*data;
	libusb_transfer	*transfer;
private:
	bool	freedata;
public:
	virtual void	callback(libusb_transfer *transfer);
private:
	virtual void	submit(libusb_device_handle *devhandle) throw(USBError);
	void	init(int length, unsigned char *data);
public:
	BulkTransfer(EndpointDescriptorPtr endpoint,
		int length, unsigned char *data = NULL);
	template<typename T>
	BulkTransfer(EndpointDescriptorPtr endpoint, const T *data) 
		: Transfer(endpoint) {
		init(sizeof(T), (unsigned char *)data);
	}
	virtual ~BulkTransfer();
	unsigned char	*getData() const;
};

/**
 * Isochronous transfers are much more complicated. In an isochronous transfer,
 * the device sends one or more packets in each microframe, as specified
 * in the EndpointDescriptor. Since a controller can only handle a relatively
 * small number of packets, the transfer has to be put together from several
 * smaller transfers, implemented by the IsoSegment class. When a transfer
 * is complete, a list of IsoPacket objects can be retrieved from the
 * segment.
 */
class	IsoTransfer;

/**
 * \brief Isochronous transfer
 *
 * A segment of an isochronous transfer is a sequence of packets received
 * from an isochronous endpoint. Since a USB controller can only handle
 * a relatively small number of packets, a larger transfer has to be 
 * split up in a sequence of smaller transfers, called IsoSegments.
 */
class IsoSegment {
	EndpointDescriptorPtr	endpoint;
	libusb_transfer	*transfer;
	IsoTransfer	*isotransfer;
public:
	IsoSegment(EndpointDescriptorPtr endpoint, int packets,
		IsoTransfer *isotransfer, libusb_device_handle *dev_handle,
		int timeout) throw(USBError);
	~IsoSegment();
	void	submit() throw(USBError);
	int	extract(std::list<std::string>& packets);
};

typedef std::shared_ptr<IsoSegment>	IsoSegmentPtr;

/**
 * \brief IsoTransfer
 *
 * The IsoTransfer object has two queues for incoming and outgoing IsoSegments
 * all segments that come out from the queue 
 */
class IsoTransfer : public Transfer {
	int	totalpackets;
	std::queue<IsoSegmentPtr>	incoming;
	std::queue<IsoSegmentPtr>	outgoing;
public:
	std::list<std::string>	packets;
private:
	std::thread	eventthread;
	std::mutex	mutex;
	std::unique_lock<std::mutex>	lock;
	std::condition_variable	condition;
public:
	virtual void	callback(libusb_transfer *transfer);
	virtual void	handlevents();
private:
	virtual void	submit(libusb_device_handle *devhandle) throw(USBError);
public:
	IsoTransfer(EndpointDescriptorPtr endpoint, int totalpackets);
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

typedef std::shared_ptr<Descriptor>	DescriptorPtr;

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

	// more detailed information about the endpoint
	typedef enum { control_transfer = 0x0, isochronous_transfer = 0x1,
		bulk_transfer = 0x2, interrupt_transfer = 0x3 } transfer_type;
	transfer_type	transferType() const;
	bool	isControl() const;
	bool	isIsochronous() const;
	bool	isBulk() const;
	bool	isInterrupt() const;
	bool	isIN() const;
	bool	isOUT() const;

	// special information for isochronous endpoints
	typedef enum { no_sync = 0x0, async_sync = 0x4, adaptive_sync = 0x8,
		synchronous_sync = 0xc } sync_type;
	sync_type	synchronizationType() const;
	typedef enum { data_usage = 0x0, feedback_usage = 0x10,
		implicit_usage = 0x20, reserved_usage = 0x30 } usage_type;
	usage_type	usageType() const;
	size_t	maxPacketSize() const;
	size_t	transactionOpportunities() const;
	size_t	maxBandwidth() const;

	// accessor to the interface
	InterfaceDescriptor&	interface();

	// text output
	std::string	toString() const;
};

typedef std::shared_ptr<EndpointDescriptor>	EndpointDescriptorPtr;

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
	Interface&	getInterface();
};

typedef	std::shared_ptr<InterfaceDescriptor>	InterfaceDescriptorPtr;

std::ostream&	operator<<(std::ostream& out, const InterfaceDescriptor& ifd);

/**
 * \brief Interface abstraction, contains all the alternate settings
 */
class Interface {
	Device&	dev;
	std::vector<InterfaceDescriptorPtr>	altsettingvector;
	int	interfaceindex;
	Configuration&	configuration;
	Interface(const Interface& other);
	bool	reattach;
public:
	Interface(Device& device, Configuration& configuration,
		const libusb_interface *li, int interface);
	~Interface();
	size_t	numAltsettings() const;
	uint8_t	interfaceNumber() const;
	const InterfaceDescriptorPtr&	operator[](size_t index) const;
	InterfaceDescriptorPtr&	operator[](size_t index);
	void	claim() throw(USBError);
	void	release() throw(USBError);
	std::string	toString() const;

	// checking whether there is a kernel driver attached to the
	// interface
	bool	kernelDriverActive() const;
	void	detachKernelDriver() throw(USBError);
	void	attachKernelDriver() const throw(USBError);
};

typedef std::shared_ptr<Interface>	InterfacePtr;

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

std::ostream&	operator<<(std::ostream& out, const USBDescriptor& descriptor);

/**
 * \brief Template function to check whether a USBDescriptorPtr is of a given
 *        type.
 *
 * \param u	USBDescriptorPtr to test for type.
 */
template<typename T>
bool	isPtr(const USBDescriptorPtr& u) {
	T	*up = dynamic_cast<T *>(&*u);
	return (NULL != up) ? true : false;
}

/**
 * \brief Template function to get a pointer to the real descriptor type
 *
 * \param u	USBDescriptorPtr to test for type.
 */
template<typename T>
T	*getPtr(const USBDescriptorPtr& u) {
	T	*up = dynamic_cast<T *>(&*u);
	if (NULL == up) {
		throw std::runtime_error("ptr of wrong type");
	}
	return up;
}

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

/**
 * \brief Frame holder class
 *
 * A helper class as intermediate step between the raw data blocks from the
 * drivers (UVC or Unicap) and the more complex FITS based image classes.
 */
class Frame : public std::string {
	int	width;
	int	height;
public:
	Frame(int width, int height);
	Frame(int width, int height, void *data, size_t length);
	int	getWidth() const;
	int	getHeight() const;
	friend class UVCCamera;
};
typedef std::shared_ptr<Frame>	FramePtr;


} // namespace usb
} // namespace astro

#endif /* _AstroUSB_h */
