/*
 * CalibrationProcess.h -- Basic process augmented by a calibration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CalibrationProcess_h
#define _CalibrationProcess_h

#include <BasicProcess.h>

namespace astro {
namespace guiding {

class CalibrationProcess : public BasicProcess {
protected:
	CalibrationPtr	_calibration;
public:
	CalibrationPtr	calibration() { return _calibration; }
	void	calibration(CalibrationPtr cal) { _calibration = cal; }
public:
	CalibrationProcess(GuiderBase *guider, TrackerPtr tracker,
                persistence::Database database = NULL);
	CalibrationProcess(const camera::Exposure& exposure,
		camera::Imager& imager, TrackerPtr tracker,
		persistence::Database database = NULL);
	void	addCalibrationPoint(const CalibrationPoint& point);
}; 

} // namespace guiding
} // namespace astro

#endif /* _CalibrationProcess_h */
