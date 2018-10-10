/*
 * Serial.h -- mixin class for serial communication
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Serial_h
#define _Serial_h

#include <string>
#include <vector>

namespace astro {
namespace device {

class Serial {
	int	fd;
	std::string	_serialdevice;
public:
	const std::string&	serialdevice() const { return _serialdevice; }
private:
	// private copy constructor to prevent copying
	Serial(const Serial&);
	Serial&	operator=(const Serial& other);
public:
	Serial(const std::string& devicename, unsigned int baudrate = 9600);
	~Serial();

	// raw byte communication
	std::vector<uint8_t>	readraw(int l);
	void	writeraw(const std::vector<uint8_t>& data);

	// text based communication
	int	write(const std::string& data);
	std::string	read(int count);
	std::string	readto(char promptchar);
};

} // namespace device
} // namespace astro

#endif /* _Serial_h */
