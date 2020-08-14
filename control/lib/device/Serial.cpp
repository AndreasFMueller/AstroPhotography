/*
 * Serial.cpp -- serial communication implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Serial.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>

namespace astro {
namespace device {

/** 
 * \brief Open a serial device
 */
Serial::Serial(const std::string& devicename, unsigned int baudrate)
	: _serialdevice(devicename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "serial device on '%s'",
		devicename.c_str());

	// initialize fd
	fd = -1;

	// first find out whether the device actually exists
	const char	*dev = devicename.c_str();
	struct stat	sb;
	if (stat(dev, &sb) < 0) {
		std::string	msg = stringprintf("device %s does not exist",
			dev);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (!(sb.st_mode & S_IFCHR)) {
		std::string	msg = stringprintf("device %s is not serial",
			dev);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// open the device and set the baud rate
	fd = open(dev, O_RDWR);
	if (fd < 0) {
		std::string	msg = stringprintf("cannot open %s: %s",
			dev, strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device opened");

	// the device must be a tty
	if (!isatty(fd)) {
		std::string	msg = stringprintf("%s is not a tty", dev);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// set basic flags
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read terminal data for %s", dev);
	struct termios	term;
	tcgetattr(fd, &term);
	term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	term.c_cflag &= ~(CSIZE | PARENB);
	term.c_cflag |= CS8 | CREAD | CLOCAL;
	term.c_oflag &= ~(OPOST);
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;

	// set baud rate
	unsigned int	speedconst;
        switch (baudrate) {
	case 50:	speedconst = B50;	break;
	case 75:	speedconst = B75;	break;
	case 110:	speedconst = B110;	break;
	case 134:	speedconst = B134;	break;
	case 150:	speedconst = B150;	break;
	case 200:	speedconst = B200;	break;
	case 300:	speedconst = B300;	break;
	case 600:	speedconst = B600;	break;
	case 1200:	speedconst = B1200;	break;
	case 2400:	speedconst = B2400;	break;
	case 4800:	speedconst = B4800;	break;
	case 9600:	speedconst = B9600;	break;
	case 19200:	speedconst = B19200;	break;
	case 38400:	speedconst = B38400;	break;
	default:
		close(fd);
		throw std::runtime_error("unknown baudrate");
		break;
	}
	cfsetispeed(&term, speedconst);
	cfsetospeed(&term, speedconst);
	if (tcsetattr(fd, TCSANOW, &term) < 0) {
		std::string	msg = stringprintf("failed to set serial "
			"line attributes: %s", strerror(errno));
		close(fd);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// check that the baud rate has been set (to detect unsupported
	// baud rates)
	if ((cfgetospeed(&term) != speedconst) ||
		(cfgetispeed(&term) != speedconst)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "serial speed NOT set");
		close(fd);
		throw std::runtime_error("serial speed not set");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "serial device ready");
}

/**
 * \brief Destroy the Serial communication instance
 *
 * This destructor closes the serial device.
 */
Serial::~Serial() {
	if (fd >= 0) {
		close(fd);
	}
	fd = -1;
}

/**
 * \brief Write a buffer of data to the serial connection
 */
int	Serial::write(const std::string& data) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	int	rc = ::write(fd, data.c_str(), data.size());
	if (rc < 0) {
		std::string	msg = stringprintf("cannot write %d bytes: %s",
					data.size(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return rc;
}

/**
 * \brief Read a number of bytes from the serial connection
 */
std::string	Serial::read(int count) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	char	buffer[count];
	int	bytes = 0;
	do {
		int	rc = ::read(fd, buffer + bytes, count - bytes);
		if (rc < 0) {
			std::string	msg = stringprintf("cannot read %d "
				"bytes: %s", count, strerror(errno));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		bytes += rc;
	} while (count > bytes);
	return std::string(buffer, count);
}

/**
 * \brief Read until we find a special character
 */
std::string	Serial::readto(char promptchar) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	std::string	result;
	char	c;
	do {
		if (1 != ::read(fd, &c, 1)) {
			throw std::runtime_error("cannot read next byte");
		}
		result.push_back(c);
	} while (c != promptchar);
	return result;
}

/**
 * \brief write a raw character array
 *
 * \param packet	the data packet to send
 */
void	Serial::writeraw(const std::vector<uint8_t>& packet) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%lu bytes to send", packet.size());
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	uint8_t	*b = (uint8_t *)alloca(packet.size());
	std::copy(packet.begin(), packet.end(), b);
	int	l = ::write(fd, b, packet.size());
	if (l != packet.size()) {
		std::string	msg = stringprintf("failed to send %u bytes: "
			"%d sent %s", packet.size(), l,
			(l < 0) ? strerror(errno) : "");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%lu bytes sent", packet.size());
}

/**
 * \brief read a number of bytes
 *
 * \param l	number of bytes to read
 */
std::vector<uint8_t>	Serial::readraw(int count) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading %d bytes", count);
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	uint8_t	buffer[count];
	int	bytes = 0;
	do {
		int	rc = ::read(fd, buffer + bytes, count - bytes);
		if (rc < 0) {
			std::string	msg = stringprintf("cannot read %d "
				"bytes: %s", count, strerror(errno));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		bytes += rc;
	} while (count > bytes);
	std::vector<uint8_t>	result;
	for (int i = 0; i < count; i++) {
		result.push_back(buffer[i]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received %lu bytes", result.size());
	return result;
}

} // namespace device
} // namespace astro
