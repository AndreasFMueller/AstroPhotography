/*
 * TaskMonitor_impl.cpp -- Implementation of task monitor
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "taskmainwindow.h"
#include <AstroDebug.h>

namespace taskmonitor {

TaskMonitor_impl::TaskMonitor_impl(TaskMainWindow& mainwindow)
	: _mainwindow(mainwindow) {
}

TaskMonitor_impl::~TaskMonitor_impl() {
}

void	TaskMonitor_impl::update(const Astro::TaskMonitorInfo& tmi) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update for %d received", tmi.taskid);
	_mainwindow.taskUpdateSlot(tmi.taskid);
}

void	TaskMonitor_impl::stop() {
}

} // namespace taskmonitor

