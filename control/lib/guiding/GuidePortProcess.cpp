/*
 * GuidePortProcess.cpp -- guider port process implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "GuidePortProcess.h"

namespace astro {
namespace guiding {

GuidePortProcess::GuidePortProcess(GuiderBase *guider,
	camera::GuidePortPtr guideport, TrackerPtr tracker,
	persistence::Database database)
	: CalibrationProcess(guider, tracker, database),
	  _guideport(guideport) {
}

GuidePortProcess::GuidePortProcess(const camera::Exposure& exposure, 
	camera::Imager& imager, camera::GuidePortPtr guideport, 
	TrackerPtr tracker, persistence::Database database)
	: CalibrationProcess(exposure, imager, tracker, database),
	  _guideport(guideport) {
}

} // namespace guiding
} // namespace astro
