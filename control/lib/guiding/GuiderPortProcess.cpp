/*
 * GuiderPortProcess.cpp -- guider port process implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BasicProcess.h>

namespace astro {
namespace guiding {

GuiderPortProcess::GuiderPortProcess(GuiderBase *guider,
	camera::GuiderPortPtr guiderport, TrackerPtr tracker,
	persistence::Database database)
	: BasicProcess(guider, tracker, database),
	  _guiderport(guiderport) {
}

GuiderPortProcess::GuiderPortProcess(const camera::Exposure& exposure, 
	camera::Imager& imager, camera::GuiderPortPtr guiderport, 
	TrackerPtr tracker, persistence::Database database)
	: BasicProcess(exposure, imager, tracker, database),
	  _guiderport(guiderport) {
}

} // namespace guiding
} // namespace astro
