/*
 * coolerupdatework.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <coolercontrollerwidget.h>

namespace snowgui {

/**
 * \brief Cooler update work construction
 */
coolerupdatework::coolerupdatework(coolercontrollerwidget *cc)
	: _coolercontrollerwidget(cc) {
}

coolerupdatework::~coolerupdatework() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy coolerupdatework");
}

/**
 * \brief Cooler update work
 *
 * This is just a slot to call the status update in the cooler instance.
 * This is not a method of the coolercontrollerwidget, but since this
 * is the only method of the update work class, it does not make sense
 * to put it into a separate file.
 */
void    coolerupdatework::statusUpdate() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
        if (_coolercontrollerwidget) {
                _coolercontrollerwidget->statusUpdate();
        }
}

} // namespace snowgui
