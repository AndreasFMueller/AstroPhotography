/*
 * Timer.cpp -- implement Timer class
 *
 * (c) 2013 Prof Dr Andreas Mueller, HOchschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {

Timer::Timer() {
	_startTime = _endTime = 0;
}

double	Timer::gettime() {
	struct timeval	t;
	gettimeofday(&t, NULL);
	return t.tv_sec + 0.000001 * t.tv_usec;
}

void	Timer::start() {
	_startTime = gettime();
}

void	Timer::end() {
	_endTime = gettime();
}

double	Timer::elapsed() {
	return _endTime - _startTime;
}

void    Timer::sleep(double t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep for %.3f seconds", t);
	unsigned int    tt = 1000000 * t;
	usleep(tt);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep complete");
}

} // namespace astro
