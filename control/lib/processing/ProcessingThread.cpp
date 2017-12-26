/*
 * ProcessingThead.cpp -- implementation of thread
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <pthread.h>
#include <AstroUtils.h>
#include <includes.h>
#include <mutex>
#include <condition_variable>

namespace astro {
namespace process {

void	start_work(ProcessingThread *t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread start");
	t->work();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread complete");
}

ProcessingThread::ProcessingThread(ProcessingStepPtr step)
	: std::thread(start_work, this), _step(step) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "barrier comming up");
	sleep(1);
	_step->_barrier.await();
}

void	ProcessingThread::work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ProcessingThread::work() start");
	_step->work();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ProcessingThread::work() end");
}

ProcessingThread::~ProcessingThread() {
}

} // namespace process
} // namespace astro
