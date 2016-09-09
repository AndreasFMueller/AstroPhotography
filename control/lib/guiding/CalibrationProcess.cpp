/*
 * CalibrationProcess.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CalibrationProcess.h>

namespace astro {
namespace guiding {

CalibrationProcess::CalibrationProcess(GuiderBase *guider, TrackerPtr tracker,
                persistence::Database database) 
	: BasicProcess(guider, tracker, database) {
}

CalibrationProcess::CalibrationProcess(const camera::Exposure& exposure,
		camera::Imager& imager, TrackerPtr tracker,
		persistence::Database database)
	: BasicProcess(exposure, imager, tracker, database) {
}

void	CalibrationProcess::addCalibrationPoint(const CalibrationPoint& point) {
	if (database()) {
		CalibrationStore	store(database());
		int	calid = calibration()->calibrationid();
		store.addPoint(calid, point);
	}
}

} // namespace guiding
} // namespace astro
