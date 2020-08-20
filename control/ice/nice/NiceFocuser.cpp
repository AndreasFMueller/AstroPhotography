/*
 * NiceFocuser.cpp -- implementation of the focuser wrapper for ICE
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceFocuser.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>

namespace astro {
namespace camera {
namespace nice {

void	NiceFocuserCallback::stop(const Ice::Current& /* current */) {
}

void	NiceFocuserCallback::movement(long fromposition, long toposition,
		 const Ice::Current& /* current */) {
	_focuser.callback(fromposition, toposition);
}

void	NiceFocuserCallback::info(long position, bool on_target,
		 const Ice::Current& /* current */) {
	_focuser.callback(position, on_target);
}



NiceFocuser::NiceFocuser(snowstar::FocuserPrx focuser,
	const DeviceName& devicename)
	: Focuser(devicename), NiceDevice(devicename), _focuser(focuser) {
	_focuser_callback = new NiceFocuserCallback(*this);
	_focuser_identity = snowstar::CommunicatorSingleton::add(
				_focuser_callback);
	_focuser->registerCallback(_focuser_identity);
}

NiceFocuser::~NiceFocuser() {
	_focuser->unregisterCallback(_focuser_identity);
	snowstar::CommunicatorSingleton::remove(_focuser_identity);
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
