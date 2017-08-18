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
	if (t < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "negative delay, "
			"return immediately");
		return;
	}
	unsigned int    tt = 1000000 * t;
	usleep(tt);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep complete");
}

std::string	Timer::timestamp(const struct timeval& tv, int resolution) {
	if (resolution > 6) {
		resolution = 6;
	}
	if (resolution < 0) {
		resolution = 0;
	}
	char	buffer[32];
	struct tm	*tmp = localtime(&tv.tv_sec);
	strftime(buffer, sizeof(buffer), "%H:%M:%S", tmp);
	int	divider = 1;
	for (int i = 0; i < 6 - resolution; i++) {
		divider *= 10;
	}
	int	sec = tv.tv_usec / divider;
	snprintf(buffer + 8, sizeof(buffer) - 8, ".%0*d", resolution, sec);
	return std::string(buffer);
}

std::string	Timer::timestamp(int resolution) {
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return timestamp(tv, resolution);
}

} // namespace astro
