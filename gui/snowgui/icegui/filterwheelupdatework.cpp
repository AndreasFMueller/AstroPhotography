/*
 * filterwheelupdatework.cpp -- Implementation of update work class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "filterwheelcontrollerwidget.h"
#include "ui_filterwheelcontrollerwidget.h"
#include <camera.h>
#include <QTimer>

namespace snowgui {

#if 0

/**
 * \brief Filterwheel update thread construction
 */
filterwheelupdatework::filterwheelupdatework(
	filterwheelcontrollerwidget *fwc)
	: _filterwheelcontrollerwidget(fwc) {
}

/**
 * \brief Destroying the filterwheel work
 */
filterwheelupdatework::~filterwheelupdatework() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying filterwheelupdatework");
}

/**
 * \brief Filterwheel update thread
 *
 * This is just a slot to call the status update in the filterwheel instance.
 * This is not a method of the filterwheelcontrollerwidget, but since this
 * is the only method of the update thread class, it does not make sense
 * to put it into a separate file.
 */
void    filterwheelupdatework::statusUpdate() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (_filterwheelcontrollerwidget) {
		_filterwheelcontrollerwidget->statusUpdate();
	}
}

/**
 * \brief Filterwheel position update
 *
 * The timer calls the position update at regular intervals. This does not
 * need to be called as frequently the statusUpdate
 */
void    filterwheelupdatework::positionUpdate() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (_filterwheelcontrollerwidget) {
		_filterwheelcontrollerwidget->positionUpdate();
	}
}

#endif

} // namespace snowgui
