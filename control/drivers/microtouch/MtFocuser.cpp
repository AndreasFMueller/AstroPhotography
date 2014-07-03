/*
 * MtFocuser.cpp -- microtouch focuser implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MtFocuser.h>
#include <AstroExceptions.h>
#include <AstroUSB.h>

using namespace astro::usb;

namespace astro {
namespace device {
namespace microtouch {

MtFocuser::MtFocuser() : Focuser(DeviceName("focuser:microtouch/focuser")) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating microtouch device");
	Context	context;
	DevicePtr	deviceptr = context.find(0x10c4, 0x82f4);
	mt = new MicroTouch(deviceptr);
}

MtFocuser::~MtFocuser() {
	delete mt;
}

unsigned short	MtFocuser::max() {
	return 60000;
}

unsigned short	MtFocuser::current() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for current position");
	return mt->position();
}

void	MtFocuser::set(unsigned short value) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got position request %hu", value);
	if (value > max()) {
		throw BadParameter("focuser value out of range");
	}
	mt->setPosition(value);
}

} // namespace microtouch
} // namespace device
} // namespace astro
