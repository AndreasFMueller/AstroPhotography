/*
 * SbigLock.cpp -- SbigLock implementation
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SbigLock.h>
#include <includes.h>
#include <mutex>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace sbig {

static std::recursive_mutex	sbigmutex;
static std::unique_lock<std::recursive_mutex>	sbiglock(sbigmutex, std::defer_lock);

SbigLock::SbigLock() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking sbig mutex");
	sbigmutex.lock();
}

SbigLock::~SbigLock() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unlocking sbig mutex");
	sbigmutex.unlock();
}

} // namespace sbig
} // namespace camera
} // namespace astro
