/*
 * CalibrationRedirector.cpp -- Callback to redirect calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CalibrationRedirector.h"

using namespace astro::callback;

namespace astro {
namespace guiding {

CallbackDataPtr	CalibrationRedirector::operator()(CallbackDataPtr data) {
	// handle the calibration
	{
		CalibrationCallbackData	*cal
			= dynamic_cast<CalibrationCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration update");
			_guider->saveCalibration();
		}
	}

	// handle progress updates
	{
		ProgressInfoCallbackData	*cal
			= dynamic_cast<ProgressInfoCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "progress update");
			_guider->calibrationProgress(cal->data().progress);
			if (cal->data().aborted) {
				_guider->forgetCalibration();
			}
		}
	}

	return data;
}

} // namespace guiding
} // namespace astro
