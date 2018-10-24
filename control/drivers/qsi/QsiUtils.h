/*
 * QsiUtils.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _QsiUtils_h
#define _QsiUtils_h

#include <AstroUtils.h>

namespace astro {
namespace camera {
namespace qsi {

#if 0

#define	START_STOPWATCH							\
	{								\
		astro::Timer stopwatch; 				\
		stopwatch.start();
#define	END_STOPWATCH(what)						\
		stopwatch.end(); 					\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s took %.3fs", what, 	\
			stopwatch.elapsed());				\
	}

#else

#define	START_STOPWATCH	
#define	END_STOPWATCH(what)

#endif

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiUtils_h */
