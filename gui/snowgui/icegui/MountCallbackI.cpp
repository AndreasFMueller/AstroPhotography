/*
 * MountCallbackI.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "mountcontrollerwidget.h"
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Construct a MountCallback interface implementation
 *
 * \param m	the mount controller to received theupdates
 */
MountCallbackI::MountCallbackI(mountcontrollerwidget& m)
	: _mountcontrollerwidget(m) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a callback %p for %p",
		this, &m);
}

/**
 * \brief Receive a state update
 *
 * \param newstate	the new state 
 * \param current	the current context for the call
 */
void    MountCallbackI::statechange(snowstar::mountstate newstate,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p status update received: %d",
		this,  newstate);
	_mountcontrollerwidget.statusUpdate();
}

/**
 * \brief Receive a position update 
 *
 * \param newposition	the new direction the telescope is pointing to
 * \param current	the current context for the call
 */
void    MountCallbackI::position(const snowstar::RaDec& newposition,
		const Ice::Current& /* current */) {
	astro::RaDec	_telescope = convert(newposition);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%p new position received: %s", this,
		_telescope.toString().c_str());
	_mountcontrollerwidget.statusUpdate();
}

} // namespace snowgui
