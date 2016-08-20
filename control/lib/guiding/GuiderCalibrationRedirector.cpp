/*
 * GuiderCalibrationRedirector.cpp -- Callback to redirect calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "GuiderCalibrationRedirector.h"

using namespace astro::callback;

namespace astro {
namespace guiding {

CallbackDataPtr	GuiderCalibrationRedirector::operator()(CallbackDataPtr data) {
	// handle the calibration
	{
		GuiderCalibrationCallbackData	*cal
			= dynamic_cast<GuiderCalibrationCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration update");
			_guider->saveCalibration(cal->data());
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
