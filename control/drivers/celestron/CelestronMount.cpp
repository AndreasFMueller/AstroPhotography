/*
 * CelestronMount.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CelestronMount.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <includes.h>
#include <stdexcept>
#include <AstroDevice.h>

namespace astro {
namespace device {
namespace celestron {

static std::string	getserialname(const std::string& devicename) {
	DeviceName	dev(devicename);
	// if the unit name is just a number, we should look up the
	// associated serial device in the properties
	try {
		int	unit = std::stoi(dev.unitname());
		Properties	properties(devicename);
		std::string	serialdevicename
			= properties.getProperty("device");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found serial device for unit %d name: %s",
			unit, serialdevicename.c_str());
		return serialdevicename;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"could not find the serial name: %s", x.what());
	}
	// if we get to this point, then the serial device path must be
	// encoded in the unit name
	return URL::decode(dev.unitname());
}

CelestronMount::CelestronMount(const std::string& devicename)
	: Mount(devicename), Serial(getserialname(devicename)) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating Celestron mount on %s",
		serialdevice().c_str());

	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// check communication
	write("Kx");
	std::string	k = read(1);
	getprompt();
	if (k != std::string("X")) {
		std::runtime_error("no echo received");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount has responded to echo request");
	// ask for version
	write("V");
	std::string	v = readto('#');
	v = v.substr(0, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "version = '%s' (%d bytes, %02X%02X)",
		v.c_str(), v.size(), v[0], v[1]);
	version = 0;
	if (v.size() >= 2) {
		try {
			version = 100 * v[0] + v[1];
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot convert: %s",
				x.what());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "version: %d", version);
}

CelestronMount::~CelestronMount() {
}

void	CelestronMount::getprompt() {
	std::string	s = read(1);
	if (s != "#") {
		throw std::runtime_error("prompt not received");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got # back");
}

Mount::state_type	CelestronMount::state() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for state command");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending J command to check alignment");
	write("J");
	std::string	s = readto('#');
	debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s (%d bytes, %02X)",
		s.c_str(), s.size(), s[0]);
	state_type	result = IDLE;
	if (s[0] == 1) {
		result = TRACKING;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending L command");
	write("L");
	s = readto('#');
	debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s (%d bytes, %02X)",
		s.c_str(), s.size(), s[0]);
	if (s == "1#") {
		result = GOTO;
	}
	return result;
}

void	CelestronMount::cancel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for cancel command");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending cancel command");
	write("M");
	getprompt();
}

void	CelestronMount::Goto(const AzmAlt& azmalt) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for GOTO command");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending GOTO AzmAtl command");
	std::string	cmd;
	if (version > 202) {
		cmd = stringprintf("b%08X,%08X",
			angle32(azmalt.azm()), angle32(azmalt.alt()));
	} else {
		cmd = stringprintf("B%04X,%04X",
			angle16(azmalt.azm()), angle16(azmalt.alt()));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command sent: %s", cmd.c_str());
	write(cmd);
	getprompt();
}

void	CelestronMount::Goto(const RaDec& radec) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for GOTO command");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending GOTO RaDec command");
	std::string	cmd;
	if (version > 106) {
		cmd = stringprintf("r%08X,%08X",
			angle32(radec.ra()), angle32(radec.dec()));
	} else {
		cmd = stringprintf("R%04X,%04X",
			angle16(radec.ra()), angle16(radec.dec()));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command sent: %s", cmd.c_str());
	write(cmd);
	getprompt();
}

std::pair<double, double>	CelestronMount::parseangles(const std::string& response) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing angle response: '%s'",
		response.c_str());
	uint32_t	a1, a2;
	if (2 != sscanf(response.c_str(), "%X,%X#", &a1, &a2)) {
		std::string	msg = stringprintf("cannot parse response '%s'",
			response.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (response.size() <= 10) {
		a1 *= 0x10000;
		a2 *= 0x10000;
	}
	return std::pair<double, double>(angle(a1), angle(a2));
}

RaDec	CelestronMount::getRaDec() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for get command");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending get RaDec command");
	if (version >= 106) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending e command");
		write("e");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending E command");
		write("E");
	}
	std::pair<double, double>	a = parseangles(readto('#'));
	if (a.second > M_PI) {
		a.second -= 2 * M_PI;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ra = %f, dec = %f",
		Angle(a.first).hours(), Angle(a.second).degrees());
	return RaDec(Angle(a.first), Angle(a.second));
}

AzmAlt	CelestronMount::getAzmAlt() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for get command");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending get AzmAlt command");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending z command");
	write("z");
	std::pair<double, double>	a = parseangles(readto('#'));
	return AzmAlt(Angle(a.first), Angle(a.second));
}

bool	CelestronMount::telescopePositionEast() {
	// XXX use the location on earth and the azm angle to find out
	// XXX on which side the telescope currently is, at least for
	// XXX GE mounts
	
#if 0
	// first query the mount to find out whether the telescope is 
	// actually equatorial by using the "t" command 
	bool	north = true; // XXX use t command

	// depending on the orientation, use the azm angle to decied whether
	// or not we are on the east/west side
	AzmAlt	azmalt = getAzmAlt();
	if (north) {
		return azmalt.azm() < 0;
	} else {
		return azmalt.azm() < 0;
	}
#endif

	// XXX until that is implemented, use the default method
	return Mount::telescopePositionEast();
}

double	CelestronMount::angle(uint16_t a) {
	return 2 * M_PI * a / 65536.;
}

double	CelestronMount::angle(uint32_t a) {
	return 2 * M_PI * a / 4294967296.;
}

uint16_t	CelestronMount::angle16(const Angle& a) {
	uint16_t	result = 65536 * a.reduced().radians() / (2 * M_PI);
	return result;
}

uint32_t	CelestronMount::angle32(const Angle& a) {
	uint32_t	result = 4294967296 * a.reduced().radians() / (2 * M_PI);
	return result;
}

} // namespace celestron
} // namespace device
} // namespace astro
