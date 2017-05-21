/*
 * SbigLock.h -- Locking for SBIG driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SbigLock_h
#define _SbigLock_h

#include <includes.h>

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief Locking class for SBIG camera driver
 */
class SbigLock {
public:
	SbigLock();
	~SbigLock();
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigLock_h */
