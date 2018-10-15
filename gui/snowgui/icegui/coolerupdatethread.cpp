/*
 * coolerupdatethread.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <coolercontrollerwidget.h>

namespace snowgui {

/**
 * \brief Cooler update thread construction
 */
coolerupdatethread::coolerupdatethread(
        coolercontrollerwidget *cc)
        : QThread(NULL), _coolercontrollerwidget(cc) {
}

/**
 * \brief Cooler update thread
 *
 * This is just a slot to call the status update in the cooler instance.
 * This is not a method of the coolercontrollerwidget, but since this
 * is the only method of the update thread class, it does not make sense
 * to put it into a separate file.
 */
void    coolerupdatethread::statusUpdate() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
        if (_coolercontrollerwidget) {
                _coolercontrollerwidget->statusUpdate();
        }
}

/**
 * \brief stop the thread
 */
void	coolerupdatethread::stop() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	_coolercontrollerwidget = NULL;
}

} // namespace snowgui
