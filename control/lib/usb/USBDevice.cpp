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
void	Device::open() throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open the device");
	// handle the case where the device has already been opened
	if (isOpen()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "device already open");
		return;
	}

	// the device is not open yet, so open it
	int	rc = libusb_open(dev, &dev_handle);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
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
Device::Device(ContextHolderPtr _context, libusb_device *_dev,
	libusb_device_handle *_dev_handle)
	: context(_context), dev(_dev), dev_handle(_dev_handle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"create a device from the libusb structure");
	// increment the reference counter
	libusb_ref_device(dev);

	// find out whether this is a broken device
	DeviceDescriptorPtr	d = descriptor();
	if (d->idVendor() == VENDOR_THE_IMAGING_SOURCE) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"broken camera: The Imaging Source");
		broken = BROKEN_THE_IMAGING_SOURCE;
	}
	if (d->idVendor() == VENDOR_ZWO) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"broken camera: ZWO ASI");
		broken = BROKEN_ZWO;
	}
}

Device::~Device() {
	close();
	libusb_unref_device(dev);
}

std::string	Device::getStringDescriptor(uint8_t index)
	const throw(USBError) {
	if (NULL == dev_handle) {
		return std::string("(device not open)");
	}
	// read the string descriptor
	unsigned char	buffer[128];
	int	rc = libusb_get_string_descriptor_ascii(dev_handle, index,
			buffer, sizeof(buffer));
	if (rc > 0) {
		return std::string((const char *)buffer, rc);
	}
	return std::string();
}

DeviceDescriptorPtr	Device::descriptor() throw(USBError) {
	// get the device descriptor
	libusb_device_descriptor	d;
	int	rc = libusb_get_device_descriptor(dev, &d);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}

	// create a DeviceDescriptor object
	DeviceDescriptor	*devdesc = new DeviceDescriptor(*this, &d);
	return DeviceDescriptorPtr(devdesc);
}

ConfigurationPtr	Device::config(uint8_t index) throw(USBError) {
	struct libusb_config_descriptor	*config = NULL;
	int	rc = libusb_get_config_descriptor(dev, index, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
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
ConfigurationPtr	Device::activeConfig() throw(USBError) {
	if (!isOpen()) {
		throw USBError("device not open");
	}
	struct libusb_config_descriptor	*config = NULL;
	int	rc = libusb_get_active_config_descriptor(dev, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
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
ConfigurationPtr	Device::configValue(uint8_t value) throw(USBError) {
	struct libusb_config_descriptor	*config;
	int	rc = libusb_get_config_descriptor_by_value(dev, value, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	Configuration	*result = new Configuration(*this, config);
	libusb_free_config_descriptor(config);
	return ConfigurationPtr(result);
}

uint8_t	Device::getBusNumber() const {
	return libusb_get_bus_number(dev);
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

void	Device::claimInterface(uint8_t interface) throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "claiming interface %d", interface);
	int	rc = libusb_claim_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot claim interface %d: %s",
			interface, libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::releaseInterface(uint8_t interface) throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "releasing interface %d", interface);
	int	rc = libusb_release_interface(dev_handle, interface);
	rc = libusb_release_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot release interface %d: %s",
			interface, libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interface released");
}

/**
 * \brief Get the number of the current configuration.
 *
 * \return configuration number
 */
int	Device::getConfiguration() throw(USBError) {
	int	result;
	int	rc = libusb_get_configuration(dev_handle, &result);
	if (rc != LIBUSB_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get configuration: %s",
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
void	Device::setConfiguration(uint8_t configuration) throw (USBError) {
	int	rc = libusb_set_configuration(dev_handle, configuration);
	if (rc != LIBUSB_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set configuration %d: %s",
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
void	Device::setInterfaceAltSetting(uint8_t interface, uint8_t altsetting)
		throw(USBError) {
	int	rc = libusb_set_interface_alt_setting(dev_handle,
			interface, altsetting);
	if (rc != LIBUSB_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set altsetting %d: %s",
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
void	Device::controlRequest(RequestBase *request) throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bmRequest = %02x, bRequest = %02x, "
		"wValue = %04x, wIndex = %04x, wLength = %d",
		request->bmRequestType(), request->bRequest(),
		request->wValue(), request->wIndex(), request->wLength());

	// for debugging, display the request content if it is going to
	// the host
	if ((request->bmRequestType() & 0x80) == RequestBase::host_to_device) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "payload to send:\n%s",
			request->payloadHex().c_str());
	}

	int	rc = libusb_control_transfer(dev_handle, 
			request->bmRequestType(),
			request->bRequest(),
			request->wValue(),
			request->wIndex(),
			request->payload(),
			request->wLength(),
			request->getTimeout());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "control request result: %d", rc);
	if (rc < 0) {
		throw USBError(libusb_error_name(rc));
	}

	// for debuggung: if the data phase goes from device to host, display
	// the response
	if ((request->bmRequestType() & 0x80) == RequestBase::device_to_host) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "payload received:\n%s",
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
void	Device::submit(Transfer *transfer) throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit transfer");
	transfer->submit(dev_handle);
}

bool	Device::kernelDriverActive(uint8_t interface) const throw(USBError) {
	int	rc = libusb_kernel_driver_active(dev_handle, interface);
	if (rc < 0) {
		throw USBError(libusb_error_name(rc));
	}
	return (rc) ? true : false;
}

void	Device::detachKernelDriver(uint8_t interface) const throw(USBError) {
	int	rc = libusb_detach_kernel_driver(dev_handle, interface);
	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot detach kernel driver: %s",
			libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::attachKernelDriver(uint8_t interface) const throw(USBError) {
	int	rc = libusb_attach_kernel_driver(dev_handle, interface);
	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot attach kernel driver: %s",
			libusb_error_name(rc));
		throw USBError(libusb_error_name(rc));
	}
}

/**
 * \brief Get a list of interface association Descriptors from the device
 */
std::list<USBDescriptorPtr>	Device::interfaceAssociationDescriptors(
					bool videoonly) throw(USBError) {
	std::list<USBDescriptorPtr>	iadescriptors;

	// see whether there is any additional data that could contain
	// an interface association descriptor
	ConfigurationPtr	config = activeConfig();
	if (config->extra().size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no data for descriptors");
		return iadescriptors;
	}

	// try to parse additional descriptors
	DescriptorFactory	f(*this);
	std::vector<USBDescriptorPtr>	list = f.descriptors(config->extra());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d additional descriptors",
		list.size());

	// no check whether they are InterfaceAssociationDescriptors
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		USBDescriptorPtr	dp = *i;
		if (isPtr<InterfaceAssociationDescriptor>(dp)) {
			InterfaceAssociationDescriptor	*iad
				= getPtr<InterfaceAssociationDescriptor>(dp);
			bool	isvideo = iad->isVideoInterfaceCollection();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "isvideo = %s",
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find out whether this is video device");
	try {
		return (interfaceAssociationDescriptors(true).size() > 0)
			? true : false;
	} catch (std::exception& x) {
		return false;
	}
}

} // namespace usb
} // namespace astro
