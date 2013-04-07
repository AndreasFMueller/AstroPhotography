/*
 * USBEndpoint.cpp -- Enddpoint descriptor implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <sstream>
#include <ios>
#include <iomanip>

namespace astro {
namespace usb {

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
	InterfaceDescriptor& _interfacedescriptor,
	const struct libusb_endpoint_descriptor *_epd) :
		Descriptor(device, _epd->extra, _epd->extra_length),
		interfacedescriptor(_interfacedescriptor) {
	copy(_epd);
}

EndpointDescriptor::~EndpointDescriptor() {
	delete epd;
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

InterfaceDescriptor&	EndpointDescriptor::interface() {
	return interfacedescriptor;
}

static std::string	indent("            ");

std::string	EndpointDescriptor::toString() const {
	std::ostringstream	out;
	out << indent << " E   bEndpointAddress:  ";
	out << std::hex << (int)bEndpointAddress() << std::endl;
	out << indent << " E   bmAttributes:     ";

	std::string	attributes;

	switch (bmAttributes() & 0x3) {
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

	if ((bmAttributes() & 0x03) == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS) {
		switch ((bmAttributes() >> 2) & 0x3) {
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

		switch ((bmAttributes() >> 4) & 0x3) {
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
	}

	out << attributes << " (";
	out << std::hex << (int)bmAttributes() << ")" << std::endl;
	out << indent << " E   wMaxPacketSize:    ";
	out << std::dec << wMaxPacketSize() << std::endl;
	out << indent << " E   bInterval:         ";
	out << std::dec << (int)bInterval() << std::endl;
	out << indent << " E   bRefresh:          ";
	out << std::dec << (int)bRefresh() << std::endl;
	out << indent << " E   bSynchAddress:     ";
	out << std::hex << (int)bSynchAddress() << std::endl;
	out << indent << "<E   extra ep data:     ";
	out << std::dec << (int)extra().size() << " bytes" << std::endl;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const EndpointDescriptor& epd) {
	return out << epd.toString();
}

} // namespace usb
} // namespace astro
