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

using namespace astro::device;

namespace astro {
namespace driver {
namespace celestron {

static std::string	getserialname(const std::string& devicename) {
	return URL::decode(DeviceName(devicename).unitname());
}

CelestronMount::CelestronMount(const std::string& devicename)
	: Mount(devicename), Serial(getserialname(devicename)) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating Celestron mount on %s",
		serialdevice().c_str());

	// check communication
	write("Kx");
	std::string	k = read(1);
	getprompt();
	if (k != std::string("X")) {
		std::runtime_error("no echo received");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount has responded");
	// ask for version
	write("V");
	std::string	v = read(2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "version = %s", v.c_str());
}

void	CelestronMount::getprompt() {
	std::string	s = read(1);
	if (s != "#") {
		throw std::runtime_error("prompt not received");
	}
}

Mount::mount_state	CelestronMount::state() {
	write("L");
	std::string	s = read(1);
	getprompt();
	return Mount::IDLE;
}

void	CelestronMount::cancel() {
	write("M");
	getprompt();
}

void	CelestronMount::Goto(const AzmAlt& azmalt) {
	std::string	cmd = stringprintf("B%08X,%08X",
		angle32(azmalt.azm()), angle32(azmalt.alt()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command sent: %s", cmd.c_str());
	write(cmd);
	getprompt();
}

void	CelestronMount::Goto(const RaDec& radec) {
	std::string	cmd = stringprintf("R%08X,%08X",
		angle32(radec.ra()), angle32(radec.dec()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command sent: %s", cmd.c_str());
	write(cmd);
	getprompt();
}

std::pair<double, double>	CelestronMount::parseangles(const std::string& response) {
	uint32_t	a1, a2;
	if (2 != sscanf(response.c_str(), "%08X,%08X#", &a1, &a2)) {
		std::string	msg = stringprintf("cannot parse response '%s'",
			response.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return std::pair<double, double>(angle(a1), angle(a2));
}

RaDec	CelestronMount::getRaDec() {
	write("e");
	std::pair<double, double>	a = parseangles(read(18));
	return RaDec(Angle(a.first), Angle(a.second));
}

AzmAlt	CelestronMount::getAzmAlt() {
	write("z");
	std::pair<double, double>	a = parseangles(read(18));
	return AzmAlt(Angle(a.first), Angle(a.second));
}

double	CelestronMount::angle(uint16_t a) {
	return a / 65536.;
}

double	CelestronMount::angle(uint32_t a) {
	return a / 4294967296.;
}

uint16_t	CelestronMount::angle16(const Angle& a) {
	uint16_t	result = 65536 * a.radians() / (2 * M_PI);
	return result;
}

uint32_t	CelestronMount::angle32(const Angle& a) {
	uint32_t	result = 4294967296 * a.radians() / (2 * M_PI);
	return result;
}

} // namespace celestron
} // namespace driver
} // namespace astro
