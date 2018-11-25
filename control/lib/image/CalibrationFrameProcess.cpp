/**
 * CalibrationFrameProcess.cpp -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <PixelValue.h>
#include <limits>
#include <AstroDebug.h>
#include <stdexcept>
#include <vector>
#include <AstroFormat.h>

using namespace astro::image;
using namespace astro::camera;
using namespace astro::adapter;

namespace astro {
namespace calibration {

void	CalibrationFrameProcess::prepare() {
	// enable cooler, set temperature, if cooler available
	bool	usecooler = (ccd->hasCooler() && (_temperature > 0));
	if (usecooler) {
		CoolerPtr	cooler = ccd->getCooler();
		cooler->setTemperature(_temperature);
		cooler->setOn(true);

		// wait until temperature is close to set point
		while (fabs(cooler->getActualTemperature() - _temperature) > 1) {
			sleep(1);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature reached");
	}
}

void	CalibrationFrameProcess::cleanup() {
	bool	usecooler = (ccd->hasCooler() && (_temperature > 0));
	if (usecooler) {
		CoolerPtr	cooler = ccd->getCooler();
		cooler->setOn(false);
	}
}

} // calibration
} // astro
