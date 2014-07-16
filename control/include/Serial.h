/*
 * Serial.h -- mixin class for serial communication
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Serial_h
#define _Serial_h

#include <string>

namespace astro {
namespace device {

class Serial {
	std::string	_serialdevice;
	int	fd;
private:
	// private copy constructor to prevent copying
	Serial(const Serial&);
	Serial&	operator=(const Serial& other);
public:
	Serial(const std::string& devicename, unsigned int baudrate = 9600);
	~Serial();
	int	write(const std::string& data);
	std::string	read(int count);
	const std::string&	serialdevice() const { return _serialdevice; }
};

} // namespace device
} // namespace astro

#endif /* _Serial_h */
