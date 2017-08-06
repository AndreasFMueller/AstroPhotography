/*
 * CalibrationImageMonitor.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <calibrationimagewidget.h>

namespace snowgui {

/**
 * \brief Construct a acalibration image Monitor
 */
CalibrationImageMonitor::CalibrationImageMonitor(calibrationimagewidget *c)
	: QObject(NULL), _calibrationimagewidget(c) {
}

/**
 * \brief Process calibration image updates
 */
void	CalibrationImageMonitor::update(const snowstar::CalibrationImageProgress& prog,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new imageno %d/%d received",
		prog.imageno, prog.imagecount);
	snowstar::CalibrationImageProgress p = prog;
	emit updateSignal(p);
}

/**
 * \brief Signals that the calibration image process is complete
 */
void	CalibrationImageMonitor::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop signal received");
	emit stopSignal();
}

} // namespace snowgui
