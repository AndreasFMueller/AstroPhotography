/*
 * BlockStopWatch.cpp -- implementation of the BlockStopWatch class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {

static double	delta(struct timeval start, struct timeval end) {
	double	result = end.tv_sec - start.tv_sec;
	result += 0.000001 * (end.tv_usec - start.tv_usec);
	return result;
}

BlockStopWatch::BlockStopWatch(const std::string& message)
	: _message(message) {
	if (debuglevel < LOG_DEBUG) {
		return;
	}
	gettimeofday(&start_time, NULL);
	getrusage(RUSAGE_SELF, &start_usage);
}

BlockStopWatch::~BlockStopWatch() {
	if (debuglevel < LOG_DEBUG) {
		return;
	}
	struct timeval	end_time;
	gettimeofday(&end_time, NULL);
	double	elapsed = delta(start_time, end_time);

	struct rusage	end_usage;
	getrusage(RUSAGE_SELF, &end_usage);
	double user = delta(start_usage.ru_utime, end_usage.ru_utime);
	double system = delta(start_usage.ru_stime, end_usage.ru_stime);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"%s: elapsed=%.6f, user=%.6f, system=%.6f", _message.c_str(),
		elapsed, user, system);
}

} // namespace astro
