/*
 * NiceFocuser.cpp -- implementation of the focuser wrapper for ICE
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceFocuser.h>

namespace astro {
namespace camera {
namespace nice {

NiceFocuser::NiceFocuser(snowstar::FocuserPrx focuser,
	const DeviceName& devicename)
	: Focuser(devicename), NiceDevice(devicename), _focuser(focuser) {
}

NiceFocuser::~NiceFocuser() {
}

unsigned short	NiceFocuser::min() {
	return _focuser->min();
}

unsigned short	NiceFocuser::max() {
	return _focuser->max();
}

unsigned short	NiceFocuser::current() {
	return _focuser->current();
}

unsigned short	NiceFocuser::backlash() {
	return _focuser->backlash();
}

void	NiceFocuser::set(unsigned short value) {
	_focuser->set(value);
}

} // namespace nice
} // namespace camera
} // namespace astro
