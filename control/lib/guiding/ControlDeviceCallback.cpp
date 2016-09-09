/*
 * ControlDeviceCallback.cpp -- implementation of ControlDevice callback
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <BasicProcess.h>
#include "ControlDeviceCallback.h"

using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief processing method to process callback 
 */
CallbackDataPtr	ControlDeviceCallback::operator()(CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"control device callback called");
	// handle calibration point upates
	{
		CalibrationPointCallbackData	*cal
			= dynamic_cast<CalibrationPointCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point: %s",
				cal->data().toString().c_str());
			return data;
		}
	}
	// handle the calibration when it completes
	{
		CalibrationCallbackData   *cal
			= dynamic_cast<CalibrationCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration update");
			_controldevice->saveCalibration();
			return data;
		}
	}
	// handle progress information
	{
		ProgressInfoCallbackData	*cal
			= dynamic_cast<ProgressInfoCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "progress update");
			if (cal->data().aborted) {
				_controldevice->calibrating(false);
			}
			return data;
		}
	}

	return data;
}

} // namespace guiding
} // namespace astro
