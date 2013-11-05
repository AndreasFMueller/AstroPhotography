/*
 * NetFocuser.cpp -- network connected focuser implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetFocuser.h>
#include <AstroUtils.h>
#include <NetUtils.h>

namespace astro {
namespace camera {
namespace net {

/**
 * \brief Create a new NetFocuser
 *
 * The constructor keeps a reference to a remote focuser object
 */
NetFocuser::NetFocuser(Astro::Focuser_var focuser)
	: Focuser(devname2netname(focuser->getName())),
	  _focuser(focuser) {
	// query the current focuser state from the remote focuser
	Astro::Focuser_Helper::duplicate(_focuser);
}

/**
 * \brief Destroy the NetFocuser
 *
 * This method releases the focuser reference
 */
NetFocuser::~NetFocuser() {
	Astro::Focuser_Helper::release(_focuser);
}

/**
 * \brief Get the minimum
 */
unsigned short	NetFocuser::min() {
	return _focuser->min();
}

/**
 * \brief Get the maximum
 */
unsigned short	NetFocuser::max() {
	return _focuser->max();
}

/**
 * \brief Get the current value
 */
unsigned short	NetFocuser::current() {
	return _focuser->current();
}

/**
 * \brief Set the focuser's set temperature
 */
void	NetFocuser::set(unsigned short _value) {
	_focuser->set(_value);
}

} // namespace net
} // namespace camera
} // namespace astro
