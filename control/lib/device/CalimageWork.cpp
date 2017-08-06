/*
 * CalimageWork.cpp -- implementation 
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImager.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {

/**
 * \brief Call the update callback if present
 */
void	CalimageWork::update() {
	if (!_callback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"no calibration image callback installed");
		return;
	}
	CalibrationImageProgress	progress;
	progress.imageno = _imageno + 1;
	progress.imagecount = _imagecount;
	CallbackDataPtr	data
		= CallbackDataPtr(new CalibrationImageProgressData(progress));
	(*_callback)(data);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sent calibration imageno = %d",
		progress.imageno);
}

/**
 * \brief Call the end callback if present
 */
void	CalimageWork::end() {
	if (!_callback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"no calibration image callback installed");
		return;
	}
	CalibrationImageProgress	progress;
	progress.imageno = -1;
	progress.imagecount = _imagecount;
	CallbackDataPtr	data
		= CallbackDataPtr(new CalibrationImageProgressData(progress));
	(*_callback)(data);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sent calibration imageno = -1");
}

} // namespace camera
} // namespace astro
