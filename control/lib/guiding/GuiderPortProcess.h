/*
 * GuiderPortProcess.h -- Processes that use a guiderport
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderPortProcess_h
#define _GuiderPortProcess_h

#include "BasicProcess.h"

namespace astro {
namespace guiding {

/**
 * \brief base process class for all processes that need a guider port
 */
class GuiderPortProcess : public BasicProcess {
	camera::GuiderPortPtr	_guiderport;
public:
	camera::GuiderPortPtr	guiderport() { return _guiderport; }

	GuiderPortProcess(GuiderBase *guider,
		camera::GuiderPortPtr guiderport, TrackerPtr tracker,
		persistence::Database database = NULL);
	GuiderPortProcess(const camera::Exposure& exposure,
		camera::Imager& imager, camera::GuiderPortPtr guiderport,
		TrackerPtr tracker, persistence::Database database = NULL);
};

} // namespace guiding
} // namespace astro

#endif /* _GuiderPortProcess_h */
