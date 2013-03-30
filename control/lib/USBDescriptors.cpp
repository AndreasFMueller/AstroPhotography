/*
 * USB.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>

namespace astro {
namespace usb {

/////////////////////////////////////////////////////////////////////
// common descriptor base class implementation
/////////////////////////////////////////////////////////////////////

Descriptor::Descriptor(Device& device, const std::string& extra)
	: dev(device), extra_descriptors(extra) {
}

Descriptor::Descriptor(const Device& device, const unsigned char *extra,
	int extra_length)
	: dev(device), extra_descriptors((const char *)extra, extra_length) {
}

Device&	Descriptor::device() { return dev; }
const Device&	Descriptor::device() const { return dev; }

const std::string&	Descriptor::extra() const {
	return extra_descriptors;
}

/////////////////////////////////////////////////////////////////////
// Endpoint Descriptor implementation
/////////////////////////////////////////////////////////////////////

void	EndpointDescriptor::copy(const struct libusb_endpoint_descriptor *_epd) {
	epd = new struct libusb_endpoint_descriptor;
	memcpy(epd, _epd, sizeof(struct libusb_endpoint_descriptor));
	epd->extra = NULL;
	epd->extra_length = 0;
}

EndpointDescriptor::EndpointDescriptor(const Device &device,
	const struct libusb_endpoint_descriptor *_epd) :
		Descriptor(device, _epd->extra, _epd->extra_length) {
	copy(_epd);
}

EndpointDescriptor&	EndpointDescriptor::operator=(const EndpointDescriptor& other) {
	memcpy(epd, other.epd, sizeof(struct libusb_endpoint_descriptor));
	return *this;
}

EndpointDescriptor::~EndpointDescriptor() {
	delete epd;
}

EndpointDescriptor::EndpointDescriptor(const EndpointDescriptor& other) 
	: Descriptor(other) {
	copy(other.epd);
}


uint8_t	EndpointDescriptor::bEndpointAddress() const {
	return epd->bEndpointAddress;
}

uint8_t	EndpointDescriptor::bmAttributes() const {
	return epd->bmAttributes;
}

uint16_t	EndpointDescriptor::wMaxPacketSize() const {
	return epd->wMaxPacketSize;
}

uint8_t	EndpointDescriptor::bInterval() const {
	return epd->bInterval;
}

uint8_t	EndpointDescriptor::bRefresh() const {
	return epd->bRefresh;
}

uint8_t	EndpointDescriptor::bSynchAddress() const {
	return epd->bSynchAddress;
}

std::ostream&	operator<<(std::ostream& out, const EndpointDescriptor& epd) {
	out << "          bEndpointAddress:  ";
	out << std::hex << (int)epd.bEndpointAddress() << std::endl;
	out << "          bmAttributes:     ";

	std::string	attributes;

	switch (epd.bmAttributes() & 0x3) {
	case LIBUSB_TRANSFER_TYPE_CONTROL:
		attributes += " control";
		break;
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
		attributes += " isochronous";
		break;
	case LIBUSB_TRANSFER_TYPE_BULK:
		attributes += " bulk";
		break;
	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		attributes += " interrupt";
		break;
	}

	switch ((epd.bmAttributes() >> 2) & 0x3) {
	case LIBUSB_ISO_SYNC_TYPE_NONE:
		attributes += " iso_sync_none";
		break;
	case LIBUSB_ISO_SYNC_TYPE_ASYNC:
		attributes += " iso_sync_async";
		break;
	case LIBUSB_ISO_SYNC_TYPE_ADAPTIVE:
		attributes += " iso_sync_adaptive";
		break;
	case LIBUSB_ISO_SYNC_TYPE_SYNC:
		attributes += " iso_sync_sync";
		break;
	}

	switch ((epd.bmAttributes() >> 4) & 0x3) {
	case LIBUSB_ISO_USAGE_TYPE_DATA:
		attributes += " iso_usage_data";
		break;
	case LIBUSB_ISO_USAGE_TYPE_FEEDBACK:
		attributes += " iso_usage_feedback";
		break;
	case LIBUSB_ISO_USAGE_TYPE_IMPLICIT:
		attributes += " iso_usage_implicit";
		break;
	}

	out << attributes << " (";
	out << std::hex << (int)epd.bmAttributes() << ")" << std::endl;
	out << "          wMaxPacketSize:    ";
	out << std::dec << epd.wMaxPacketSize() << std::endl;
	out << "          bInterval:         ";
	out << std::dec << (int)epd.bInterval() << std::endl;
	out << "          bRefresh:          ";
	out << std::dec << (int)epd.bRefresh() << std::endl;
	out << "          bSynchAddress:     ";
	out << std::hex << (int)epd.bSynchAddress() << std::endl;
	return out;
}

/////////////////////////////////////////////////////////////////////
// Interface Descriptor implementation
/////////////////////////////////////////////////////////////////////

void	InterfaceDescriptor::copy(const struct libusb_interface_descriptor *_ifdp) {
	ifdp = new struct libusb_interface_descriptor;
	memcpy(ifdp, _ifdp, sizeof(struct libusb_interface_descriptor));
	ifdp->extra = NULL;
	ifdp->extra_length = 0;
}

InterfaceDescriptor::InterfaceDescriptor(const Device& device,
	const struct libusb_interface_descriptor *_ifdp)
		: Descriptor(device, _ifdp->extra, _ifdp->extra_length) {
	copy(_ifdp);
	getEndpoints();
}

InterfaceDescriptor::InterfaceDescriptor(const InterfaceDescriptor& other) :
	Descriptor(other) {
	copy(other.ifdp);
	endpoints = other.endpoints;
}

InterfaceDescriptor&	InterfaceDescriptor::operator=(const InterfaceDescriptor& other) {
	memcpy(ifdp, other.ifdp, sizeof(struct libusb_interface_descriptor));
	endpoints = other.endpoints;
	return *this;
}

uint8_t	InterfaceDescriptor::bInterfaceNumber() const {
	return ifdp->bInterfaceNumber;
}

uint8_t	InterfaceDescriptor::bAlternateSetting() const {
	return ifdp->bAlternateSetting;
}

uint8_t	InterfaceDescriptor::bInterfaceClass() const {
	return ifdp->bInterfaceClass;
}

uint8_t	InterfaceDescriptor::bInterfaceSubClass() const {
	return ifdp->bInterfaceSubClass;
}

uint8_t	InterfaceDescriptor::bInterfaceProtocol() const {
	return ifdp->bInterfaceProtocol;
}

const std::list<EndpointDescriptor>&	InterfaceDescriptor::endpoint() const {
	return endpoints;
}

void	InterfaceDescriptor::getEndpoints() {
	for (int i = 0; i < ifdp->bNumEndpoints; i++) {
		EndpointDescriptor	ep(device(), &ifdp->endpoint[i]);
		endpoints.push_back(ep);
	}
}

std::ostream&	operator<<(std::ostream& out, const InterfaceDescriptor& ifd) {
	out << "      bInterfaceNumber:      ";
	out << (int)ifd.bInterfaceNumber() << std::endl;
	out << "      bAlternateSetting:     ";
	out << (int)ifd.bAlternateSetting() << std::endl;
	out << "      bInterfaceClass:       ";
	out << (int)ifd.bInterfaceClass() << std::endl;
	out << "      bInterfaceSubClass:    ";
	out << (int)ifd.bInterfaceSubClass() << std::endl;
	out << "      bInterfaceProtocol:    ";
	out << (int)ifd.bInterfaceProtocol() << std::endl;
	const std::list<EndpointDescriptor>&	eplist = ifd.endpoint();
	out << "        " << eplist.size() << " Endpoints:" << std::endl;
	std::list<EndpointDescriptor>::const_iterator	i;
	int	endpointno = 0;
	for (i = eplist.begin(); i != eplist.end(); i++, endpointno++) {
		out << "        Endpoint " << endpointno << ":" << std::endl;
		out << *i;
	}
	return out;
}

/////////////////////////////////////////////////////////////////////
// Interface implementation
/////////////////////////////////////////////////////////////////////

Interface::Interface(const Device& device, const libusb_interface *li)
	: dev(device) {
	for (int i = 0; i < li->num_altsetting; i++) {
		InterfaceDescriptor	id(device, &li->altsetting[i]);
		altsetting.push_back(id);
	}
}

const std::list<InterfaceDescriptor>&	Interface::altsettings() const {
	return altsetting;
}

std::ostream&	operator<<(std::ostream& out, const Interface& interface) {
	out << "      Interface with " << interface.altsettings().size();
	out << " alternate settings:" << std::endl;;
	const std::list<InterfaceDescriptor>&	idref = interface.altsettings();
	std::list<InterfaceDescriptor>::const_iterator	i;
	for (i = idref.begin(); i != idref.end(); i++) {
		out << *i;
	}
	return out;
}

/////////////////////////////////////////////////////////////////////
// Configuration Descriptors
/////////////////////////////////////////////////////////////////////

void	ConfigDescriptor::copy(const libusb_config_descriptor *_config) {
	config = new libusb_config_descriptor;
	memcpy(config, _config, sizeof(libusb_config_descriptor));
	config->extra = NULL;
	config->extra_length = 0;
}

ConfigDescriptor::ConfigDescriptor(const Device& device,
	const struct libusb_config_descriptor *config)
	: Descriptor(device, config->extra, config->extra_length) {
	copy(config);
	getInterfaces();
}

ConfigDescriptor::ConfigDescriptor(const ConfigDescriptor& other)
	: Descriptor(other) {
	copy(other.config);
	interfaces = other.interfaces;
}

ConfigDescriptor&	ConfigDescriptor::operator=(const ConfigDescriptor& other) {
	memcpy(config, other.config, sizeof(libusb_config_descriptor));
	interfaces = other.interfaces;
}

uint8_t ConfigDescriptor::bConfigurationValue() const {
	return config->bConfigurationValue;
}

uint8_t ConfigDescriptor::bNumInterfaces() const {
	return config->bNumInterfaces;
}

const std::list<Interface>&	ConfigDescriptor::interface() const {
	return interfaces;
}

void	ConfigDescriptor::getInterfaces() {
	for (int i = 0; i < config->bNumInterfaces; i++) {
		Interface	ifd(device(), &config->interface[i]);
		interfaces.push_back(ifd);
	}
}

std::ostream&	operator<<(std::ostream& out, const ConfigDescriptor& config) {
	out << "    bConfigurationValue:    ";
	out << (int)config.bConfigurationValue() << std::endl;
	out << "    bNumInterfaces:         ";
	const std::list<Interface>&	l = config.interface();
	out << (int)config.bNumInterfaces() << "(" << l.size() << ")" << std::endl;
	std::list<Interface>::const_iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		out << *i;
	}
	return out;
}

} // namespace usb
} // namespace astro
