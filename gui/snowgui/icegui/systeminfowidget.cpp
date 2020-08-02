/*
 * systeminfowidget.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswi
 */
#include "systeminfowidget.h"
#include "ui_systeminfowidget.h"
#include <types.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroTypes.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>


namespace snowgui {

/**
 * \brief Construct a SystemInfoWidget 
 *
 * \param parent	the parent widget
 */
SystemInfoWidget::SystemInfoWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::SystemInfoWidget) {
	ui->setupUi(this);

	connect(&_timer, SIGNAL(timeout()),
		this, SLOT(update()));

	_timer.setInterval(1000);
}

/**
 * \brief Destroy the SystemInfoWidget
 */
SystemInfoWidget::~SystemInfoWidget() {
	_timer.stop();
	delete ui;
}

/**
 * \brief Static global SystemInfoWidget
 *
 * The global SystemInfoWidget can be used to request heartbeat
 * updates to other widgets so they can reconnect when the state
 * changes.
 */
static SystemInfoWidget	*_systeminfowidget = NULL;

/**
 * \brief get a global systeminfowidget
 */
SystemInfoWidget	*SystemInfoWidget::global() {
	return _systeminfowidget;
}

/**
 * \brief Remember a systeinfowidget as the global one
 *
 * \param s	the SystemInfoWidget to remember
 */
void	SystemInfoWidget::setGlobal(SystemInfoWidget *s) {
	_systeminfowidget = s;
}

/**
 * \brief Connect to a daemon
 *
 * \param daemon	the daemon to use for system info
 */
void	SystemInfoWidget::setDaemon(snowstar::DaemonPrx daemon) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add daemon proxy");
	_timer.stop();
	_daemon = daemon;
	if (_daemon) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "starting timer");
		_timer.start();

		// construct the heartbeat monitor
		snowgui::HeartbeatMonitor	*heartbeatmonitor
			= new snowgui::HeartbeatMonitor();
		_heartbeat_monitor = heartbeatmonitor;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "heartbeat monitor created: %p",
			heartbeatmonitor);
		add_receiver(this);
		// get the identity
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get identity");
		_heartbeat_identity = snowstar::CommunicatorSingleton::add(
			_heartbeat_monitor);
		// register the monitor
		_daemon->registerHeartbeatMonitor(_heartbeat_identity);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "monitor registered");

		// get the heartbeat interval
		ui->heartWidget->interval(_daemon->heartbeatInterval());
	}
}

/**
 * \brief Handle an udpate
 */
void	SystemInfoWidget::update() {
	if (!_daemon) {
		return;
	}
	// daemon uptime
	try {
		QString	uptimestring(astro::stringprintf("%.2f",
			_daemon->daemonUptime()).c_str());
		ui->daemonUptimeField->setText(uptimestring);
	} catch (...) {
	}
	// temperature
	try {
		QString temperaturestring(astro::stringprintf("%.1f°C",
			_daemon->getTemperature() - astro::Temperature::zero).c_str());
		ui->temperatureField->setText(temperaturestring);
	} catch (...) {
	}
	// cpu time
	try {
		QString	cputimestring(astro::stringprintf("%.2fs",
			_daemon->cputime()).c_str());
		ui->cputimeField->setText(cputimestring);
	} catch (...) {
	}
	// sysinfo
	try {
		snowstar::Sysinfo	sysinfo = _daemon->getSysinfo();
		// load
		QString	loadstring(astro::stringprintf("%d/%d/%d",
			sysinfo.load1min, sysinfo.load5min,
			sysinfo.load15min).c_str());
		ui->loadField->setText(loadstring);
		// systemmemory
		QString	memorystring(astro::stringprintf("used %.0f, free %.0f, buffers %.0f",
			(sysinfo.totalram - sysinfo.freeram) / (1024. * 1024.),
			(sysinfo.freeram) / (1024. * 1024.),
			(sysinfo.bufferram) / (1024. * 1024.)).c_str());
		ui->systemMemoryField->setText(memorystring);
	} catch (...) {
	}
	// process size
	try {
		float	s = _daemon->processSize();
		QString	sizestring(astro::stringprintf("%.3f MiB", s / (1024.*1024.)).c_str());
		ui->sizeField->setText(sizestring);
	} catch (...) {
	}
}

/**
 * \brief handle heartbeat updates
 */
void	SystemInfoWidget::heartbeat_update(QString s) {
	ui->heartbeatField->setText(s);
	ui->heartWidget->beat();
}

/**
 * \brief Handle notifications that the heartbeat was lost
 */
void	SystemInfoWidget::heartbeat_lost() {
	if (!_daemon) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot do anything about it");
		return;
	}
	ui->heartWidget->dead();
	try {
		_daemon->unregisterHeartbeatMonitor(_heartbeat_identity);
#if 0
		snowstar::CommunicatorSingleton::remove(_heartbeat_identity);
		_heartbeat_identity = snowstar::CommunicatorSingleton::add(
			_heartbeat_monitor);
#endif
		_daemon->registerHeartbeatMonitor(_heartbeat_identity);
	} catch (const std::exception& x) {
	}
}

/**
 * \brief Handle notifications that the heartbeat was reconnected
 */
void	SystemInfoWidget::heartbeat_reconnected() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reconnected");
}

} // namespace snowgui
