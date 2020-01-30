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

namespace snowgui {

SystemInfoWidget::SystemInfoWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::SystemInfoWidget) {
	ui->setupUi(this);

	connect(&_timer, SIGNAL(timeout()),
		this, SLOT(update()));

	_timer.setInterval(1000);
}

SystemInfoWidget::~SystemInfoWidget() {
	_timer.stop();
	delete ui;
}

void	SystemInfoWidget::setDaemon(snowstar::DaemonPrx daemon) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add daemon proxy");
	_timer.stop();
	_daemon = daemon;
	if (_daemon) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "starting timer");
		_timer.start();
	}
}

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
}


} // namespace snowgui
