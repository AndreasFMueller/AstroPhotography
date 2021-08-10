/*
 * USBDevice.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <USBDebug.h>

using namespace astro;

namespace astro {
namespace usb {

void	Device::getDescriptor(struct libusb_device_descriptor *devdesc) const {
	libusb_get_device_descriptor(dev, devdesc);
}

bool	Device::isOpen() const {
	return (NULL == dev_handle) ? false : true;
}

/**
 * \brief Open the device.
 *
 * Most operations need that a USB device that is open. During a bus
 * scan, we may want to look at a device that is not open, so the default
 * is to just get a device, but not to open it. This method also opens
 * the device.
 */
void	Device::open() {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "open the device");
	// handle the case where the device has already been opened
	if (isOpen()) {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "device already open");
		return;
	}

	// the device is not open yet, so open it
	int	rc = libusb_open(dev, &dev_handle);
	if (rc != LIBUSB_SUCCESS) {
		std::string	msg = stringprintf("cannot open device: %s",
			libusb_error_name(rc));
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}
}

/**
 * \brief Close a device.
 *
 * This method closes the device. This does not precluded it may
 * later be reopened.
 */
void	Device::close() {
	if (NULL == dev_handle) {
		return;
	}
	libusb_close(dev_handle);
	dev_handle = NULL;
}

/**
 * \brief Create the device.
 *
 * This private constructor is used to build a Device object from 
 * the libusb data structure.
 *
 * \param _dev		The libusb_device structure to use for the
 *			Device.
 * \param _dev_handle	The libusb_device_handle structure to use if
 *			the device is already open.
 */
Device::Device(ContextHolderPtr _context, libusb_device *_dev)
	: context(_context), dev(_dev), dev_handle(NULL) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0,
		"create a device bus=%d, port=%d",
		getBusNumber(), getPortNumber());

	// increment the reference counter
	libusb_ref_device(dev);

	// get the device descriptor
	int	rc = libusb_get_device_descriptor(dev, &devdesc);
	if (rc != LIBUSB_SUCCESS) {
		std::string	msg = stringprintf("cannot get device "
			"descriptor: %s", libusb_error_name(rc));
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}

	// we need to open the device, otherwise we cannot read the
	// string descriptors
	open();

	// find out whether this is a broken device
	switch (getVendorId()) {
	case VENDOR_THE_IMAGING_SOURCE:
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0,
			"broken camera: The Imaging Source");
		broken = BROKEN_THE_IMAGING_SOURCE;
		break;
	case VENDOR_ZWO:
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0,
			"broken camera: ZWO ASI");
		broken = BROKEN_ZWO;
		break;
	default:
		break;
	}
}

Device::~Device() {
	close();
	libusb_unref_device(dev);
}

// try at most three times to get the string descriptor. This is used
// as a workaround for a problem with the at90usb162 microcontroller
// used in the othello devices which cases the string transfer to fail
static const int	max_retries = 3;

/**
 * \brief Get a string descriptor
 *
 * \param index	Index of the string descriptor to retrieve
 */
std::string	Device::getStringDescriptor(uint8_t index) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve string descriptor %d from %p",
		index, dev_handle);
	if (0 == index) {
		std::string	msg = stringprintf("0 not a valid string index",
			index);
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}
	if (NULL == dev_handle) {
		std::string	msg = stringprintf("cannot get String %d: "
			"device not open", index);
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}
	// read the string descriptor
	unsigned char	buffer[128];
	int	retries = 0;
	int	rc;
	do {
		rc = libusb_get_string_descriptor_ascii(dev_handle,
				index, buffer, sizeof(buffer));
		if (rc > 0) {
			std::string	result((const char *)buffer, rc);
			USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "got string %d: '%s'",
				index, result.c_str());
			return result;
		}
	} while (max_retries > ++retries);

	// fake the strings for SX devices
	if (getVendorId() == 0x1278) {
		return stringprintf("string%d", index);
	}

	std::string	msg = stringprintf("cannot get string %d: %s (%d)",
		index, libusb_error_name(rc), rc);
	USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw USBError(msg);
}

/**
 * \brief Get the descriptor of the device
 */
DeviceDescriptorPtr	Device::descriptor() {
	// create a DeviceDescriptor object
	DeviceDescriptor	*Devdesc = new DeviceDescriptor(*this);
	return DeviceDescriptorPtr(Devdesc);
}

/**
 * \brief Get a configuration for a given index
 *
 * \param index	index of the configuration to retrieve
 */
ConfigurationPtr	Device::config(uint8_t index) {
	struct libusb_config_descriptor	*config = NULL;
	int	rc = libusb_get_config_descriptor(dev, index, &config);
	if (rc != LIBUSB_SUCCESS) {
		std::string	msg = stringprintf("cannot get config %d: %s",
			index, libusb_error_name(rc));
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}
	Configuration	*result = new Configuration(*this, config);
	libusb_free_config_descriptor(config);
	return ConfigurationPtr(result);
}

/**
 * \brief Get the context from which this device was created.
 *
 * The context is needed for event handling. Unfortunately, libusb
 * does not give any methods to retrieve the context from the libusb_device
 * or the libusb_handle, although it apparently keeps references to the
 * context internally.
 */
ContextHolderPtr	Device::getContext() const {
	return context;
}

/**
 * \brief Get active configuaration descriptor.
 *
 * This method returns the active device descriptor. The device has to
 * be open for this to work. This is a restriction imposed by a bug in
 * libusb-1.0: on Mac OS X, the library causes a segmentation fault when
 * trying to retrieve the active configuration descriptor of a device that
 * was not opened. Although it works on other platforms (e.g. Linux),
 * by enforcing this restriction on all platforms we ensure that code
 * in Linux cannot inadvertently trigger this bug and cause segementation
 * faults on Mac OS X.
 */
ConfigurationPtr	Device::activeConfig() {
	if (!isOpen()) {
		throw USBError("device not open");
	}
	struct libusb_config_descriptor	*config = NULL;
	int	rc = libusb_get_active_config_descriptor(dev, &config);
	if (rc != LIBUSB_SUCCESS) {
		std::string	msg = stringprintf("cannot active config: %s",
			libusb_error_name(rc));
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}
	Configuration	*result = new Configuration(*this, config);
	libusb_free_config_descriptor(config);
	return ConfigurationPtr(result);
}

/**
 * \brief Get configuration by value.
 *
 * \param value	configuration value to search for.
 */
ConfigurationPtr	Device::configValue(uint8_t value) {
	struct libusb_config_descriptor	*config;
	int	rc = libusb_get_config_descriptor_by_value(dev, value, &config);
	if (rc != LIBUSB_SUCCESS) {
		std::string	msg = stringprintf("cannot get config value "
			"%d: %s", value, libusb_error_name(rc));
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}
	Configuration	*result = new Configuration(*this, config);
	libusb_free_config_descriptor(config);
	return ConfigurationPtr(result);
}

uint8_t	Device::getBusNumber() const {
	return libusb_get_bus_number(dev);
}

uint8_t	Device::getPortNumber() const {
	return libusb_get_port_number(dev);
}

uint8_t	Device::getDeviceAddress() const {
	return libusb_get_device_address(dev);
}

std::string	Device::getDeviceName() const {
	return stringprintf("%03d-%03d", (int)getBusNumber(), (int)getDeviceAddress());
}

/**
 * \brief Get code indicating in what respect the device is broken.
 *
 * Some devices have broken USB descriptors or handle the protocol
 * in a broken way. To simplify working around such problems, there
 * is code in the constructor that detects such cases. This method
 * then allows an application program to get this code an provide
 * workarounds for the deficiencies of the device.
 */
int	Device::getBroken() const {
	return broken;
}

void	Device::claimInterface(uint8_t interface) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "claiming interface %d", interface);
	int	rc = libusb_claim_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot claim interface %d: %s",
			interface, libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::releaseInterface(uint8_t interface) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "releasing interface %d", interface);
	int	rc = libusb_release_interface(dev_handle, interface);
	rc = libusb_release_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot release interface %d: %s",
			interface, libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "interface released");
}

/**
 * \brief Get the number of the current configuration.
 *
 * \return configuration number
 */
int	Device::getConfiguration() {
	int	result;
	int	rc = libusb_get_configuration(dev_handle, &result);
	if (rc != LIBUSB_SUCCESS) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot get configuration: %s",
			libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
	return result;
}

/**
 * \brief Select a configuration by number
 *
 * \param configuration	number of the configuration to select.
 */
void	Device::setConfiguration(uint8_t configuration) {
	int	rc = libusb_set_configuration(dev_handle, configuration);
	if (rc != LIBUSB_SUCCESS) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot set configuration %d: %s",
			configuration, libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

/**
 * \brief Select an alternate setting on an interface.
 *
 * 
 * \param interface	interface number
 * \param altsetting	number of alternate setting
 */
void	Device::setInterfaceAltSetting(uint8_t interface, uint8_t altsetting) {
	int	rc = libusb_set_interface_alt_setting(dev_handle,
			interface, altsetting);
	if (rc != LIBUSB_SUCCESS) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot set altsetting %d: %s",
			altsetting, libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

/**
 * \brief Execute a control request.
 *
 * All information necessary to execute a control request to the device
 * is contained in the request argument. This method just sends the
 * contents of the request to the device. If the request includes
 * a data phase, then the direction of the data phase was encoded in
 * the request when the request was constructed.
 *
 * \param request	pointer to the control request
 */
void	Device::controlRequest(RequestBase *request) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "processing %p", request);
	if (NULL == request) {
		std::runtime_error("no request structure for control request");
	}
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "bmRequest = %02x, bRequest = %02x, "
		"wValue = %04x, wIndex = %04x, wLength = %d",
		(unsigned int)request->bmRequestType(),
		(unsigned int)request->bRequest(),
		(unsigned int)request->wValue(),
		(unsigned int)request->wIndex(),
		(unsigned int)request->wLength());

	// for debugging, display the request content if it is going to
	// the host
	if ((request->bmRequestType() & 0x80) == RequestBase::host_to_device) {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "payload to send:\n%s",
			request->payloadHex().c_str());
	}

	// preparing the control transfer
	if (NULL == dev_handle) {
		std::string	msg = stringprintf("%hx/%hx has no handle",
			getVendorId(), getProductId());
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	int	rc = libusb_control_transfer(dev_handle, 
			request->bmRequestType(),
			request->bRequest(),
			request->wValue(),
			request->wIndex(),
			request->payload(),
			request->wLength(),
			request->getTimeout());
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "control request result: %d", rc);
	if (rc < 0) {
		throw USBError(libusb_error_name(rc));
	}

	// for debuggung: if the data phase goes from device to host, display
	// the response
	if ((request->bmRequestType() & 0x80) == RequestBase::device_to_host) {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "payload received:\n%s",
			request->payloadHex().c_str());
	}

	// only accept the response if it has the right length, otherwise
	// throw an exception
	if ((rc != request->wLength()) && (!request->accept_short_response)) {
		std::string	message
			= stringprintf("expecting %d bytes, %d received",
				request->wLength(), rc);
		std::cerr << request->payloadHex();
		throw USBError(message.c_str());
	}
}

enum Device::usb_speed	Device::getDeviceSpeed() const {
	return (Device::usb_speed)libusb_get_device_speed(dev);
}

uint16_t	Device::getVendorId() {
	return devdesc.idVendor;
}

uint16_t	Device::getProductId() {
	return devdesc.idProduct;
}

std::ostream&	operator<<(std::ostream& out, const Device& device) {
	out	<< "bus " << (int)device.getBusNumber()
		<< " address " << (int)device.getDeviceAddress()
		<< " speed " << (int)device.getDeviceSpeed() << std::endl;
	return out;
}

int	Device::maxIsoPacketSize(uint8_t endpoint) const {
	return libusb_get_max_iso_packet_size(dev, endpoint);
}

/**
 * \brief Submit a transfer to a device.
 *
 * The transfers are really handled by the Transfer classes themselves,
 * but the Device class has to supply the device handle to the transfer
 * class.
 * \param transfer	A Transfer instance.
 */
void	Device::submit(Transfer *transfer) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "submit transfer");
	transfer->submit(dev_handle);
}

bool	Device::kernelDriverActive(uint8_t interface) const {
	int	rc = libusb_kernel_driver_active(dev_handle, interface);
	if (rc < 0) {
		throw USBError(libusb_error_name(rc));
	}
	return (rc) ? true : false;
}

void	Device::detachKernelDriver(uint8_t interface) const {
	int	rc = libusb_detach_kernel_driver(dev_handle, interface);
	if (rc < 0) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot detach kernel driver: %s",
			libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::attachKernelDriver(uint8_t interface) const {
	int	rc = libusb_attach_kernel_driver(dev_handle, interface);
	if (rc < 0) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot attach kernel driver: %s",
			libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

/**
 * \brief Get a list of interface association Descriptors from the device
 */
std::list<USBDescriptorPtr>	Device::interfaceAssociationDescriptors(
					bool videoonly) {
	std::list<USBDescriptorPtr>	iadescriptors;

	// see whether there is any additional data that could contain
	// an interface association descriptor
	ConfigurationPtr	config = activeConfig();
	if (config->extra().size() == 0) {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "no data for descriptors");
		return iadescriptors;
	}

	// try to parse additional descriptors
	DescriptorFactory	f(*this);
	std::vector<USBDescriptorPtr>	list = f.descriptors(config->extra());
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "found %d additional descriptors",
		list.size());

	// no check whether they are InterfaceAssociationDescriptors
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		USBDescriptorPtr	dp = *i;
		if (isPtr<InterfaceAssociationDescriptor>(dp)) {
			InterfaceAssociationDescriptor	*iad
				= getPtr<InterfaceAssociationDescriptor>(dp);
			bool	isvideo = iad->isVideoInterfaceCollection();
			USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "isvideo = %s",
				(isvideo) ? "YES" : "NO");
			if ((!videoonly) || (isvideo)) {
				iadescriptors.push_back(dp);
			}
		}
	}

	// return all the interface association descriptors we have found
	return iadescriptors;
}

/**
 * \brief Find out whether this is a video device
 */
bool	Device::isVideoDevice() {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "find out whether this is video device");
	try {
		return (interfaceAssociationDescriptors(true).size() > 0)
			? true : false;
	} catch (std::exception& x) {
		return false;
	}
}

} // namespace usb
} // namespace astro
