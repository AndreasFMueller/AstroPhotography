/*
 * SxGuiderPort.cpp -- guider port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxGuiderPort.h>
#include <AstroDebug.h>
#include <algorithm>
#include <SxUtils.h>

namespace astro {
namespace camera {
namespace sx {

//////////////////////////////////////////////////////////////////////
// time spec class implementation
//////////////////////////////////////////////////////////////////////
void	timespec::normalize() {
	while (ts.tv_nsec >= 1000000000) {
		ts.tv_nsec -= 1000000000;
		ts.tv_sec += 1;
	}
}

timespec::timespec() {
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = 1000 * tv.tv_usec;
}

timespec::timespec(double when) {
	if (when <= 0) {
		ts.tv_sec = 0;
		ts.tv_nsec = 0;
		return;
	}
	ts.tv_sec = trunc(when);
	ts.tv_nsec = 1000000000 * (when - ts.tv_sec);
}

timespec::timespec(struct ::timespec& when) {
	ts.tv_sec = when.tv_sec;
	ts.tv_nsec = when.tv_nsec;
	normalize();
}

timespec::timespec(struct timeval& when) {
	ts.tv_sec = when.tv_sec;
	ts.tv_nsec = 1000 * when.tv_usec;
	normalize();
}

timespec::timespec(const timespec& other) {
	ts.tv_sec = other.ts.tv_sec;
	ts.tv_nsec = other.ts.tv_nsec;
}

timespec	timespec::operator+(const timespec& other) const {
	struct ::timespec	result;
	result.tv_sec = ts.tv_sec + other.ts.tv_sec;
	result.tv_nsec = ts.tv_nsec + other.ts.tv_nsec;
	return timespec(result);
}

timespec	timespec::operator+(const double& other) const {
	struct ::timespec	result;
	result.tv_sec = ts.tv_sec + trunc(other);
	result.tv_nsec = ts.tv_nsec + 1000000000 * (other - trunc(other));
	return timespec(result);
}

bool	timespec::operator<(const timespec& other) const {
	if (ts.tv_sec < other.ts.tv_sec) { return true; }
	if ((ts.tv_sec == other.ts.tv_sec) && (ts.tv_nsec < other.ts.tv_nsec)) {
		return true;
	}
	return false;
}

std::string	timespec::toString() const {
	char	b[32];
	snprintf(b, sizeof(b), "%ld.%09ld", ts.tv_sec, ts.tv_nsec);
	return std::string(b);
}

#define	SX_RAPLUS_BIT	1
#define SX_DECPLUS_BIT	2
#define SX_DECMINUS_BIT	4
#define	SX_RAMINUS_BIT	8

static void	*sxguidermain(void *private_data) {
	SxGuiderPort	*guiderport = (SxGuiderPort *)private_data;
	return guiderport->main();
}

SxGuiderPort::SxGuiderPort(SxCamera& _camera)
	: GuiderPort(GuiderPort::defaultname(_camera.name(), "guiderport")),
	  camera(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating a guider port");
	// initialize tze turnoff variables
	for (int i = 0; i < 4; i++) { turnoff.push_back(timespec(0)); }
	current = 0;
	cancel = false;

	// initialize mutex and condition variable
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	pthread_cond_init(&condition, NULL);

	// start the worker thread
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	if (pthread_create(&thread, &attr, sxguidermain, this)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "failed to create guider thread");
		throw SxError("cannot create guider thread");
	}

	// new we can release the guider port thread
	pthread_mutex_unlock(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construction compmlete, "
		"release guider port thread");
}

SxGuiderPort::~SxGuiderPort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelling guider thread");
	cancel = true;
	pthread_cond_signal(&condition);

	// wait for the thread to terminate
	void	*result;
	pthread_join(thread, &result);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread terminated");

	// clean up the inter thread communication variables
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&condition);
}

/**
 * \brief thread main function
 *
 * The main function of the guider port thread waits until either 
 * one of the turnoff variables expires or a state change was signaled 
 * from the activate method.
 */
void	*SxGuiderPort::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[%p] main function", pthread_self());

	// we lock the mutex, thus blocking the thread until we want to
	// release it.
	pthread_mutex_lock(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "[%p] guider port thread released",
		pthread_self());

	while (true) {
		// find the current time
		timespec	now;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%p] current time: %s",
			pthread_self(), now.toString().c_str());

		// forward 0.01 seconds
		now = now + 0.01;

		// find out which bits should still be active
		uint8_t	newstate = 0;
		if (now < turnoff[0]) {
			newstate |= SX_RAPLUS_BIT;
		}
		if (now < turnoff[1]) {
			newstate |= SX_RAMINUS_BIT;
		}
		if (now < turnoff[2]) {
			newstate |= SX_DECPLUS_BIT;
		}
		if (now < turnoff[3]) {
			newstate |= SX_DECMINUS_BIT;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%p] new port state: %02x",
			pthread_self(), newstate);

		// find the time for the next event
		timespec	next = now + 1000000;
		std::vector<timespec>::const_iterator	i;
		for (i = turnoff.begin(); i != turnoff.end(); i++) {
			if ((*i < next) && (now < *i)) {
				next = *i;
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%p] next event at %d.%09d",
			pthread_self(), next.ts.tv_sec, next.ts.tv_nsec);

		// now set the new state
		current = newstate;
		if (cancel) {
			current = 0;
		}
		EmptyRequest	request(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, 0,
			(uint8_t)SX_CMD_SET_STAR2K, (uint16_t)current);
		camera.controlRequest(&request);

		// if cancelled, we turminate now
		if (cancel) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "[%p] thread cancelled",
				pthread_self());
			return NULL;
		}

		// wait for timeout or condition, this also releases the
		// mutex lock as a side effect. When this method returns,
		// the mutex is again locked.
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%p] wait for next event",
			pthread_self());
		pthread_cond_timedwait(&condition, &mutex, &next.ts);
	}
	
	return NULL;
}

uint8_t	SxGuiderPort::active() {
	uint8_t	result = 0;
	pthread_mutex_lock(&mutex);
	result |= (current & SX_RAPLUS_BIT) ? RAPLUS : 0;
	result |= (current & SX_RAMINUS_BIT) ? RAMINUS : 0;
	result |= (current & SX_RAPLUS_BIT) ? DECPLUS : 0;
	result |= (current & SX_RAMINUS_BIT) ? DECMINUS : 0;
	pthread_mutex_unlock(&mutex);
	return result;
}

void	SxGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "activate(%f, %f, %f, %f)",
		raplus, raminus, decplus, decminus);
	// first acquire the mutex so that we can write the request variable
	if (pthread_mutex_lock(&mutex)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot lock mutex");
		throw SxError("cannot lock mutex");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mutex locked");

	// now we can write the new turnoff times
	turnoff[0] = timespec() + raplus;
	turnoff[1] = timespec() + raminus;
	turnoff[2] = timespec() + decplus;
	turnoff[3] = timespec() + decminus;

	// release the mutex
	pthread_mutex_unlock(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mutex unlocked, signaling condition");

	// signal that some things have changed
	pthread_cond_signal(&condition);
}

} // namespace sx
} // namespace camera
} // namespace astro
