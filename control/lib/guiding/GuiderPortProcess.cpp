/*
 * GuiderPortProcess.cpp -- guider port process implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "GuiderPortProcess.h"

namespace astro {
namespace guiding {

GuiderPortProcess::GuiderPortProcess(GuiderBase *guider,
	camera::GuiderPortPtr guiderport, TrackerPtr tracker,
	persistence::Database database)
	: CalibrationProcess(guider, tracker, database),
	  _guiderport(guiderport) {
}

GuiderPortProcess::GuiderPortProcess(const camera::Exposure& exposure, 
	camera::Imager& imager, camera::GuiderPortPtr guiderport, 
	TrackerPtr tracker, persistence::Database database)
	: CalibrationProcess(exposure, imager, tracker, database),
	  _guiderport(guiderport) {
}

} // namespace guiding
} // namespace astro
