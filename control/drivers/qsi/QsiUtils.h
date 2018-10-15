/*
 * QsiUtils.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>

namespace astro {
namespace camera {
namespace qsi {

#define	START_STOPWATCH							\
	astro::Timer stopwatch; 					\
	stopwatch.start();
#define	END_STOPWATCH(what)						\
	stopwatch.end(); 						\
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s took %.3fs", what, 		\
		stopwatch.elapsed());

} // namespace qsi
} // namespace camera
} // namespace astro
