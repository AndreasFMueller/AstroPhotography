/*
 * mountupdatework.cpp -- work on updating the mount information
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <mountcontrollerwidget.h>

namespace snowgui {

/**
 * \brief Construt a mountupdatework object
 */
mountupdatework::mountupdatework(mountcontrollerwidget *mc)
	: _mountcontrollerwidget(mc) {
}

/**
 * \brief Destroy the work object
 */
mountupdatework::~mountupdatework() {
}

/**
 * \brief Slot called by the timer to do the status update
 *
 * This slot just calls the statusUpdate method of the mount controller
 */
void	mountupdatework::statusUpdate() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (_mountcontrollerwidget) {
		_mountcontrollerwidget->statusUpdate();
	}
}

} // namespace snowgui
