/*
 * AstroDevice.h -- Device manager
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDevice_h
#define _AstroDevice_h

#include <string>

namespace astro {
namespace device {

class Device {
protected:
	std::string	_name;
public:
	Device(const std::string& name = "") : _name(name) { }
	const std::string&	getName() const { return _name; }
};

} // namespace device
} // namespace astro

#endif /* _AstroDevice_h */
