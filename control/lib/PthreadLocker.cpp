/*
 * PthreadLocker.cpp -- implementation of the locker utility class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <includes.h>
#include <stdexcept>

namespace astro {

/**
 * \brief Construct a locker object
 *
 * The block argument specifies whether we should block on the lock or
 * throw an exception if the lock cannot be acquired.
 */
PthreadLocker::PthreadLocker(pthread_mutex_t *lock, bool blocking)
	: _lock(lock) {
	if (blocking) {
		if (pthread_mutex_lock(lock)) {
			throw std::runtime_error(std::string("cannot lock ")
				+ std::string(strerror(errno)));
		}
	} else {
		if (pthread_mutex_trylock(lock)) {
			throw std::runtime_error("already locked");
		}
	}
}

/**
 * \brief Unlock the lock
 */
PthreadLocker::~PthreadLocker() {
	pthread_mutex_unlock(_lock);
}

} // namespace astro
