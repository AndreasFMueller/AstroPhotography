/*
 * SxAO.cpp -- implementation of the SX AO interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "SxAO.h"
#include <includes.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {
namespace sx {

SxAO::SxAO(const DeviceName& name) : AdaptiveOptics(name) {
	_hasguideport = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create AO on device %s",
		name.toString().c_str());

	Properties	properties(name);
	if (!properties.hasProperty("device")) {
		std::string	cause = stringprintf(
			"serial device for %s not defined",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw NotFound(cause);
	}
	device = properties.getProperty("device");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using serial device %s",
		device.c_str());
	initialize(device);
	center();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AO unit created");
}

SxAO::~SxAO() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "closing serial interface");
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
	char	result = response();
	if (result == 'G') { return true; }
	if (result == 'L') { return false; }
	throw std::runtime_error("incorrect response from AO unit");
}

char	SxAO::response() {
	char	result;
	if (1 != read(serial, &result, 1)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "serial error: %s",
			strerror(errno));
		throw std::runtime_error("cannot get a response");
	}
	return result;
}

bool	SxAO::move2(int x, int y) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "move2(%d, %d)", x, y);
	char	ewchar = (x > 0) ? 'W' : 'T';
	char	nschar = (y > 0) ? 'N' : 'S';
	char	cmd[15];
	snprintf(cmd, sizeof(cmd), "G%c%05dG%c%05d",
		ewchar, abs(x), nschar, abs(y));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "goto command: %s", cmd);
	if (14 != write(serial, cmd, 14)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot write command %s: %s",
			cmd, strerror(errno));
		throw std::runtime_error("cannot write move command");
	}
	char	r[3];
	memset(r, 0, sizeof(r));
	r[0] = response();
	r[1] = response();
	if ((r[0] == 'G') && (r[1] == 'G')) {
		return true;
	}
	if ((r[0] != 'G') || (r[0] != 'L')) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s", r);
		throw std::runtime_error("bad response for East-West move");
	}
	if ((r[1] != 'G') || (r[1] != 'L')) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s", r);
		throw std::runtime_error("bad response for North-South move");
	}
	if (r[0] == 'L') {
		debug(LOG_ERR, DEBUG_LOG, 0, "east-west jam");
	}
	if (r[1] == 'L') {
		debug(LOG_ERR, DEBUG_LOG, 0, "north-south jam");
	}
	return false;
}

bool	SxAO::findcenter() {
	if (1 != write(serial, "K", 1)) {
		throw std::runtime_error("cannot write center command");
	}
	char	r = response();
	if ('K' != r) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"incorrect response from find centre command");
		throw std::runtime_error("cannot find center");
	}
	return true;
}

void	SxAO::initialize(const std::string& serialdevice) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initializing AO unit");
	// open the serial device
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening serial port %s",
		serialdevice.c_str());
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
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot initialize AO unit: %s",
			x.what());
		if (serial >= 0) {
			close(serial);
			serial = -1;
		}
		throw x;
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
#if 1
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
#else
	if ((x == 0) && (y == 0)) {
		return;
	}
	if ((x == 0) && (y != 0)) {
		if (y > 0) {
			north(y);
		} else {
			south(-y);
		}
		return;
	} 
	if ((x != 0) && (y == 0)) {
		if (x > 0) {
			west(x);
		} else {
			east(-x);
		}
	}
	move2(x, y);
#endif
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
	char	result = response();
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
