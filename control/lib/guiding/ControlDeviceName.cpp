/*
 * ControlDeviceName.cpp -- control device name implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

ControlDeviceName::ControlDeviceName(const GuiderName& guidername,
	ControlDeviceType type) : GuiderName(guidername), _type(type) {
}

ControlDeviceName::ControlDeviceName(const ControlDeviceName& other)
	: GuiderName(other), _type(other._type) {
}

ControlDeviceName&	ControlDeviceName::operator=(const ControlDeviceName& other) {
	GuiderName::operator=(other);
	_type = other.type;
	return *this;
}

ControlDeviceType	ControlDeviceName::controldevicetype() const {
	return _type;
}

void	ControlDeviceName::controldevicetype(ControlDeviceType t) {
	_type = t;
}

void	ControlDeviceName::checktype(ControlDeviceType t) {
	if (_type != t) {
		std::string	msg = stringprintf("control device type "
			"mismatch %s != %s", type2string(_type).c_str(),
			type2string(t).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

} // namespace guiding
} // namespace astro
