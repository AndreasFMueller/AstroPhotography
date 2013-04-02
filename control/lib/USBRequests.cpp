/*
 * USBRequests.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <sstream>
#include <iomanip>

using namespace astro::usb;

namespace astro {
namespace usb {

Request::Request(const usb_request_header_t *_header) {
	header = (usb_request_header_t *)new uint8_t[8 + _header->wLength];
	memcpy(header, _header, 8 + _header->wLength);
}

Request::~Request() {
	delete[] header;
}

uint8_t	*Request::payload() const {
	return 8 + (uint8_t *)header;
}

uint16_t	Request::wLength() const {
	return header->wLength;
}

void	Request::copyTo(void *data) {
	memcpy(data, header, 8 + wLength());
}

std::string	Request::toString() const {
	std::ostringstream	out;
	out << "bmRequestType: ";
	out << std::hex << (int)header->bmRequestType << std::endl;
	out << "bRequest:      ";
	out << std::hex << (int)header->bRequest << std::endl;
	out << "wValue:        ";
	out << std::hex << header->wValue << std::endl;
	out << "wIndex:        ";
	out << std::hex << header->wIndex << std::endl;
	out << "wLength:       ";
	out << std::dec << header->wLength;
	uint8_t	*data = 8 + (uint8_t *)header;
	for (int i = 0; i < header->wLength; i++) {
		if (0 == (i % 16)) {
			out << std::endl;
		}
		out << " ";
		out << std::hex << std::setw(2) << std::setfill('0');
		out << (int)data[i];
	}
	out << std::endl;
	return out.str();
}

} // namespace usb
} // namespace astro
