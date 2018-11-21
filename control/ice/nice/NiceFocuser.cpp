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

long	NiceFocuser::min() {
	return _focuser->min();
}

long	NiceFocuser::max() {
	return _focuser->max();
}

long	NiceFocuser::current() {
	return _focuser->current();
}

long	NiceFocuser::backlash() {
	return _focuser->backlash();
}

void	NiceFocuser::set(long value) {
	_focuser->set(value);
}

} // namespace nice
} // namespace camera
} // namespace astro
