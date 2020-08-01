/*
 * HeartbeatMonitor.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswi
 */
#include "systeminfowidget.h"
#include <types.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroTypes.h>

namespace snowgui {

HeartbeatMonitor::HeartbeatMonitor() : QObject(NULL) {
}

HeartbeatMonitor::~HeartbeatMonitor() {
}

void	HeartbeatMonitor::beat(int sequence_number,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sequence_number = %d", sequence_number);
	time_t	now;
	time(&now);
	char	buffer[128];
	struct tm	*timep = localtime(&now);
	strftime(buffer, sizeof(buffer), "%T %F", timep);
	std::string	l = astro::stringprintf("%s, seqno = %d", buffer,
		sequence_number);
	emit update(QString(l.c_str()));
}

void	HeartbeatMonitor::stop(const Ice::Current& /* current */) {
}


} // namespace snowgui
