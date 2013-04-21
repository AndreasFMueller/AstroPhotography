/*
 * USBEndpoint.cpp -- Enddpoint descriptor implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <sstream>
#include <ios>
#include <iomanip>
#include <cassert>
#include <string.h>

namespace astro {
namespace usb {

/////////////////////////////////////////////////////////////////////
// Endpoint Descriptor implementation
/////////////////////////////////////////////////////////////////////

void	EndpointDescriptor::copy(const struct libusb_endpoint_descriptor *_epd) {
	epd = new struct libusb_endpoint_descriptor;
	assert(NULL != epd);
	memcpy(epd, _epd, sizeof(struct libusb_endpoint_descriptor));
	epd->extra = NULL;
	epd->extra_length = 0;
}

EndpointDescriptor::EndpointDescriptor(Device& device,
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

EndpointDescriptor::transfer_type	EndpointDescriptor::transferType() const {
	return (transfer_type)(bmAttributes() & 0x3);
}

size_t	EndpointDescriptor::maxPacketSize() const {
	return (wMaxPacketSize() & 0x7ff);
}

size_t	EndpointDescriptor::transactionOpportunities() const {
	return 1 + (0x3 & (wMaxPacketSize() >> 11));
}

bool	EndpointDescriptor::isControl() const {
	return (transferType() == control_transfer) ? true : false;
}

bool	EndpointDescriptor::isIsochronous() const {
	return (transferType() == isochronous_transfer) ? true : false;
}

bool	EndpointDescriptor::isBulk() const {
	return (transferType() == bulk_transfer) ? true : false;
}

bool	EndpointDescriptor::isInterrupt() const {
	return (transferType() == interrupt_transfer) ? true : false;
}

EndpointDescriptor::sync_type	EndpointDescriptor::synchronizationType() const {
	return (sync_type)((0x3 << 2) & bmAttributes());
}

EndpointDescriptor::usage_type	EndpointDescriptor::usageType() const {
	return (usage_type)((0x3 << 4) & bmAttributes());
}

/**
 * \brief Maximum number of databytes that can be transferred per second.
 *
 * This method uses the information in the interface descriptor to compute
 * the maximum number of bytes that can be transferred per second on this
 * endpoint during an isochronous transfer.
 * The method returns 0 for other types of endpoints.
 */
size_t	EndpointDescriptor::maxBandwidth() const {
	size_t	bandwidth = 1000 * transactionOpportunities() * maxPacketSize();
	switch (dev.getDeviceSpeed()) {
	case Device::SPEED_HIGH:
	case Device::SPEED_SUPER:
		bandwidth *= 8;
		break;
	case Device::SPEED_UNKNOWN:
	case Device::SPEED_LOW:
	case Device::SPEED_FULL:
		break;
	}
	return bandwidth;
}

static std::string	indent("            E   ");

std::string	EndpointDescriptor::toString() const {
	std::ostringstream	out;
	out << indent << "bEndpointAddress:  ";
	out << std::hex << (int)bEndpointAddress() << std::endl;
	out << indent << "bmAttributes:     ";

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

	out << indent << "wMaxPacketSize:    ";
	out << std::dec << transactionOpportunities();
	out << " x ";
	out << std::dec << maxPacketSize() << std::endl;

	out << indent << "bInterval:         ";
	out << std::dec << (int)bInterval() << std::endl;
	out << indent << "bRefresh:          ";
	out << std::dec << (int)bRefresh() << std::endl;
	out << indent << "bSynchAddress:     ";
	out << std::hex << (int)bSynchAddress() << std::endl;
	out << indent << "extra EP data:     ";
	out << std::dec << (int)extra().size() << " bytes" << std::endl;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const EndpointDescriptor& epd) {
	return out << epd.toString();
}

} // namespace usb
} // namespace astro
