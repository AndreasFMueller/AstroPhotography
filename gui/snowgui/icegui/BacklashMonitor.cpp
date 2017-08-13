/*
 * BacklashMonitor.cpp -- implementation of backlash monitor
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <BacklashMonitor.h>
#include <backlashdialog.h>
#include <AstroDebug.h>

namespace snowgui {

BacklashMonitor::BacklashMonitor(BacklashDialog *bd) : _backlashdialog(bd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "BacklashMonitor created for %p", bd);
	qRegisterMetaType<snowstar::BacklashPoint>("snowstar::BacklashPoint");
	qRegisterMetaType<snowstar::BacklashResult>("snowstar::BacklashResult");
}

void	BacklashMonitor::updatePoint(const snowstar::BacklashPoint& point,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updatePoint callback: %.1f,%.1f",
		point.xoffset, point.yoffset);
	emit updatePointSignal(point);
}

void	BacklashMonitor::updateResult(const snowstar::BacklashResult& result,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updateResult callback");
	emit updateResultSignal(result);
}

void	BacklashMonitor::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop callback");
	emit stopSignal();
}

} // namespace snowgui
