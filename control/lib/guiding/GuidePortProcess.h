/*
 * GuidePortProcess.h -- Processes that use a guideport
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuidePortProcess_h
#define _GuidePortProcess_h

#include "CalibrationProcess.h"

namespace astro {
namespace guiding {

/**
 * \brief base process class for all processes that need a guider port
 */
class GuidePortProcess : public CalibrationProcess {
	camera::GuidePortPtr	_guideport;
public:
	camera::GuidePortPtr	guideport() { return _guideport; }

	GuidePortProcess(GuiderBase *guider,
		camera::GuidePortPtr guideport, TrackerPtr tracker,
		persistence::Database database = NULL);
	GuidePortProcess(const camera::Exposure& exposure,
		camera::Imager& imager, camera::GuidePortPtr guideport,
		TrackerPtr tracker, persistence::Database database = NULL);
};

} // namespace guiding
} // namespace astro

#endif /* _GuidePortProcess_h */
