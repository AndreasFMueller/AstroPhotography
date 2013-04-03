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

std::string	RequestBase::toString() const {
	std::ostringstream      out;

	// display the request header
	out << "bmRequesetType: "; 
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
	uint8_t	*data = this->payload();
	for (int i = 0; i < this->wLength(); i++) {
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
