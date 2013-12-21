/*
 * SxAO.cpp -- implementation of the SX AO interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxAO.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace sx {

static astro::DeviceName	aoname(const std::string& name) {
	DeviceName	modulename("module:sx");
	DeviceName	a(modulename, DeviceName::AdaptiveOptics, name);
	return a;
}

SxAO::SxAO(const std::string& name) : AdaptiveOptics(aoname(name)) {
	_hasguiderport = false;
	initialize("/dev/cu.KeySerial1");
	center();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AO unit created");
}

SxAO::~SxAO() {
	close(serial);
}

bool	SxAO::east(int steps) {
	bool	result = move('T', steps);
	offset[0] += steps;
	return result;
}

bool	SxAO::west(int steps) {
	bool	result = move('W', steps);
	offset[0] -= steps;
	return result;
}

bool	SxAO::north(int steps) {
	bool	result = move('N', steps);
	offset[1] += steps;
	return result;
}

bool	SxAO::south(int steps) {
	bool	result = move('S', steps);
	offset[1] -= steps;
	return result;
}

bool	SxAO::move(char d, int steps) {
	if (steps < 0) {
		throw std::runtime_error("steps must be positive");
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%c move", d);
	char	cmd[8] = "G 00001";
	cmd[1] = d;
	snprintf(cmd + 2, 6, "%05d", steps);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", cmd);
	if (7 != write(serial, cmd, 7)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write command %s: %s",
			cmd, strerror(errno));
		throw std::runtime_error("cannot write move command");
	}
	char	result;
	if (1 != read(serial, &result, 1)) {
		throw std::runtime_error("cannot get a response");
	}
	if (result == 'G') { return true; }
	if (result == 'L') { return false; }
	throw std::runtime_error("incorrect response from AO unit");
}

bool	SxAO::findcenter() {
	if (1 != write(serial, "K", 1)) {
		throw std::runtime_error("cannot write center command");
	}
	char	response;
	int	bytes;
	if (1 != (bytes = read(serial, &response, 1))) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot read response: %d",
			bytes);
	}
	if ('K' != response) {
		throw std::runtime_error("cannot find center");
	}
	return true;
}

void	SxAO::initialize(const std::string& serialdevice) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initializing AO unit");
	// open the serial device
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening serial port");
	serial = -1;
	try {
		serial = open(serialdevice.c_str(), O_RDWR | O_NOCTTY);
		if (serial < 0) {
			throw std::runtime_error("cannot open serial device");
		}
		if (!isatty(serial)) {
			close(serial);
			throw std::runtime_error("serial is not a tty");
		}

		// initialize the serial port
		debug(LOG_DEBUG, DEBUG_LOG, 0, "initializing serial port");
		struct termios	term;
		tcgetattr(serial, &term);
		term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
		term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
		term.c_cflag &= ~(CSIZE | PARENB);
		term.c_cflag |= CS8 | CREAD | CLOCAL;
		term.c_oflag &= ~(OPOST);
		term.c_cc[VMIN] = 1;
		term.c_cc[VTIME] = 0;
		cfsetispeed(&term, B9600);
		cfsetospeed(&term, B9600);
		if (tcsetattr(serial, TCSANOW, &term) < 0) {
			debug(LOG_ERR, DEBUG_LOG, 0, "serial setup fails: %s",
				strerror(errno));
			close(serial);
			throw std::runtime_error("cannot initialize the serial port");
		}

		// center the AO unit
		debug(LOG_DEBUG, DEBUG_LOG, 0, "centering");
		if (!findcenter()) {
			throw std::runtime_error("cannot find center");
		}
		offset[0] = offset[1] = 0;

		// find maximum and minimum values for each direction
		debug(LOG_DEBUG, DEBUG_LOG, 0, "find maximum");
		limits[0] = limits[1] = 50;

	} catch (std::exception& x) {
		if (serial >= 0) {
			close(serial);
			serial = -1;
		}
	}
}

void	SxAO::set0(const Point& position) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"setting to %s position (current %f, %f)",
		position.toString().c_str(),
		offset[0] / (double)limits[0], offset[1] / (double)limits[1]);
	int	x, y;
	x = lround((position.x() - (offset[0] / (double)limits[0])) * limits[0]);
	y = lround((position.y() - (offset[1] / (double)limits[1])) * limits[1]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correct: %d, %d", x, y);
	if (x > 0) {
		east(x);
	}
	if (x < 0) {
		west(-x);
	}
	if (y > 0) {
		north(y);
	}
	if (y < 0) {
		south(-y);
	}
}

bool	SxAO::mountmove(char d, int steps) {
	char	cmd[8];
	snprintf(cmd, sizeof(cmd), "M%c%05d", d, steps);
	if (7 != write(serial, cmd, 7)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"could not send mount move cmd: %s",
			strerror(errno));
		throw std::runtime_error("could not send mount move command");
	}
	char	result;
	if (1 != read(serial, &result, 1)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"could not read mount move response: %s",
			strerror(errno));
		throw std::runtime_error("could not receive mount move response");
	}
	return 'M' == result;
}

bool	SxAO::decplus(int steps) {
	return mountmove('N', steps);
}

bool	SxAO::decminus(int steps) {
	return mountmove('S', steps);
}

bool	SxAO::raplus(int steps) {
	return mountmove('W', steps);
}

bool	SxAO::raminus(int steps) {
	return mountmove('T', steps);
}

} // namespace sx
} // namespace camera
} // namespace astro
