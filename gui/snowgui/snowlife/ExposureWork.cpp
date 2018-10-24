/*
 * ExposureWork.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "liveview.h"

namespace snowgui {

/**
 * \brief Work class to encapsulate the exposure work in a different thread
 *
 * \param liveview	the liveview MainWindow containing the 
 */
ExposureWork::ExposureWork(LiveView *liveview)
	: QObject(NULL), _liveview(liveview) {
}

/**
 * \brief Destroy the ExposureWork object
 */
ExposureWork::~ExposureWork() {
}

/**
 * \brief Perform the exposure work
 *
 * Note that this simply calls the doExposure() method of the LiveView
 * instance. But since ExposureWork is in a different thread, the exposure
 * work is done in a different thread from the main thread.
 */
void	ExposureWork::doExposure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ExposureWork::doExposure() started");
	_liveview->doExposure();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ExposureWork::doExposure() terminated");
}

} // namespace snowgui
