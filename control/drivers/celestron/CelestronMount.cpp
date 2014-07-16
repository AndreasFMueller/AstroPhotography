/*
 * CelestronMount.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CelestronMount.h>
#include <AstroDebug.h>
#include <AstroUtils.h>

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
}

void	CelestronMount::Goto(const RaDec& radec) {
}

RaDec	CelestronMount::getRaDec() {
}

AzmAlt	CelestronMount::getAzmAlt() {
}


} // namespace celestron
} // namespace driver
} // namespace astro
