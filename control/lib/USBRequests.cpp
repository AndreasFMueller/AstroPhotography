/*
 * USBRequests.cpp -- control request abstraction
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <sstream>
#include <iomanip>

using namespace astro::usb;

namespace astro {
namespace usb {

//////////////////////////////////////////////////////////////////////
// RequestBase implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Initialize the request.
 *
 * This copies the data provided to the payload structure, if there is
 * anything to copy. If the request is from device to host, then it also
 * clears the payload data.
 */
void	RequestBase::init_data(void *data) {
	if ((this->bmRequestType() & 0x80) == host_to_device) {
		// for requests with data phase to the device, copy the
		// data
		if (NULL == data) {
			return;
		}
		memcpy(this->payload(), data, this->wLength());
	} else {
		// for requests in the reverse direction, initialize the
		// data buffer to zero
		if (this->wLength()) {
			memset(this->payload(), 0, this->wLength());
		}
	}
}

RequestBase::RequestBase(request_type _type, EndpointDescriptorPtr endpoint,
	void *data) : type(_type) {
	recipient = endpoint_recipient;
	direction = (NULL != data) ? host_to_device : device_to_host;
	bEndpointAddress = 0x1f & endpoint->bEndpointAddress();
	accept_short_response = false;
}

RequestBase::RequestBase(request_type _type, InterfacePtr interface,
	void *data) : type(_type) {
	recipient = interface_recipient;
	direction = (NULL != data) ? host_to_device : device_to_host;
	bInterface = interface->interfaceNumber();
	accept_short_response = false;
}

RequestBase::RequestBase(request_type _type, request_recipient _recipient,
	void *data) : type(_type) {
	recipient = _recipient;
	direction = (NULL != data) ? host_to_device : device_to_host;
	accept_short_response = false;
}

uint16_t	RequestBase::wIndex() const {
	switch (recipient) {
	case endpoint_recipient:
		return direction | bEndpointAddress;
		break;
	case interface_recipient:
		return bInterface;
		break;
	default:
		return 0;
	}
}

uint8_t	RequestBase::bmRequestType() const {
	return direction | type | recipient;
}

std::string	RequestBase::toString() const {
	std::ostringstream      out;

	// display the request header
	out << "bmRequestType:  "; 
	out << std::hex << std::setw(2) << std::setfill('0'); 
	out << (int)this->bmRequestType() << std::endl;

	out << "bRequest:       ";
	out << std::hex << std::setw(2) << std::setfill('0');
	out << (int)this->bRequest() << std::endl;

	out << "wValue:         ";
	out << std::hex << std::setw(4) << std::setfill('0');
	out << this->wValue() << std::endl;

	out << "wIndex:         ";
	out << std::hex << std::setw(4) << std::setfill('0');
	out << this->wIndex() << std::endl;

	out << "wLength:        " << std::dec;
	out << this->wLength() << std::endl;

	// display the contents as hex
	out << payloadHex();
	return out.str();
}

std::string	RequestBase::payloadHex() const {
	std::ostringstream	out;
	// display the contents as hex
	uint8_t	*data = this->payload();
	for (int i = 0; i < this->wLength(); i++) {
		int	col = i % 16;
		switch (col) {
		case 0:
			if (i > 0) {
				out << std::endl;
			}
			out << std::hex << std::setw(4) << std::setfill('0');
			out << i << "   ";
			break;
		case 8:
			out << " ";
		default:
			out << " ";
		}
		out << std::hex << std::setw(2) << std::setfill('0');
		out << (int)data[i];
	}
	out << std::endl;
	return out.str();
}

void	EmptyRequest::setwLength(int length) {
	header.wLength = length;
}

//////////////////////////////////////////////////////////////////////
// EmptyRequest implementation
//////////////////////////////////////////////////////////////////////

void	EmptyRequest::init(uint8_t bRequest, uint16_t wValue) {
	header.bmRequestType = this->bmRequestType();
	header.bRequest = bRequest;
	header.wValue = wValue;
	header.wLength = 0;
	header.wIndex = RequestBase::wIndex();
}

EmptyRequest::EmptyRequest(request_type type,
	const EndpointDescriptorPtr endpoint, uint8_t bRequest,
	uint16_t wValue) 
	: RequestBase(type, endpoint, NULL) {
	init(bRequest, wValue);
}

EmptyRequest::EmptyRequest(request_type type,
	const InterfacePtr interface, uint8_t bRequest, uint16_t wValue)
	: RequestBase(type, interface, NULL) {
	init(bRequest, wValue);
}

EmptyRequest::EmptyRequest(request_type type, request_recipient recipient,
		 uint16_t wIndex, uint8_t bRequest, uint16_t wValue)
	: RequestBase(type, recipient, NULL) {
	init(bRequest, wValue);
	header.wIndex = wIndex;
}

uint8_t	EmptyRequest::bmRequestType() const {
	// make sure empty requests are always considered as
	// host-to-device requests
	return 0x7f & RequestBase::bmRequestType();
}

uint8_t EmptyRequest::bRequest() const {
	return header.bRequest;
}

uint16_t        EmptyRequest::wValue() const {
	return header.wValue;
}

uint16_t        EmptyRequest::wIndex() const {
	return header.wIndex;
}

uint16_t        EmptyRequest::wLength() const {
	return 0;
}

uint8_t *EmptyRequest::payload() const {
	return NULL;
}

} // namespace usb
} // namespace astro
