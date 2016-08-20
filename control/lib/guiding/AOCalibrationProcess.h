/*
 * AOCalibrationProcess.h -- 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AOCalibrationProcess_h
#define _AOCalibrationProcess_h

#include <BasicProcess.h>

namespace astro {
namespace guiding {

/**
 * \brief Calibration of an adaptive optics 
 */
class AOCalibrationProcess : public BasicProcess {
	camera::AdaptiveOpticsPtr	_adaptiveoptics;
public:
	camera::AdaptiveOpticsPtr	adaptiveoptics() {
		return _adaptiveoptics;
	}

	// constructors
public:
	AOCalibrationProcess(GuiderBase *guider,
		camera::AdaptiveOpticsPtr adaptiveoptics, TrackerPtr tracker,
		persistence::Database database = NULL);

	void	main(astro::thread::Thread<AOCalibrationProcess>& thread);

	// callbacks
	void	callback(const CalibrationPoint& calpoint);
        void	callback(const ProgressInfo& progressinfo);
        void	callback(const GuiderCalibration& calibration);
        void	callback(const ImagePtr& image);

};

} // namespace guiding
} // namespace astro

#endif /* _AOCalibrationProcess_h */
