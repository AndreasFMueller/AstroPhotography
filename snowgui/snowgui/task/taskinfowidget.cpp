/*
 * taskinfowidget.cpp -- implementation of task info widget
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "taskinfowidget.h"
#include "ui_taskinfowidget.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Construct a task info widget
 */
taskinfowidget::taskinfowidget(QWidget *parent)
	: QDialog(parent), ui(new Ui::taskinfowidget) {
	ui->setupUi(this);
	_taskid = -1;

	connect(ui->refreshButton, SIGNAL(clicked()),
		this, SLOT(refreshClicked()));
	connect(ui->imageButton, SIGNAL(clicked()),
		this, SLOT(imageClicked()));
	connect(ui->closeButton, SIGNAL(clicked()),
		this, SLOT(closeClicked()));
}

/**
 * \brief Destroy the task info widget
 */
taskinfowidget::~taskinfowidget() {
	delete ui;
}

/**
 * \brief Set up the ICE communication with various services and monitor
 */
void	taskinfowidget::setProxies(snowstar::TaskQueuePrx tasks,
		snowstar::ImagesPrx images,
		snowstar::RepositoriesPrx repositories) {
	_tasks = tasks;
	_images = images;
	_repositories = repositories;

	// initialize the task monitor
	_taskmonitor = new TaskMonitorController(NULL);
	_taskmonitorptr = Ice::ObjectPtr(_taskmonitor);
	_taskmonitor->setTasks(_tasks, _taskmonitorptr);

	// connect the task monitor to this widget
	connect(_taskmonitor, SIGNAL(taskUpdate(snowstar::TaskMonitorInfo)),
			this, SLOT(taskUpdate(snowstar::TaskMonitorInfo)));
}

/**
 * \brief Slot to udpate the task information
 */
void	taskinfowidget::updateTask(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update task %d", taskid);
	if (!_tasks) {
		return;
	}
	snowstar::TaskInfo	info;
	snowstar::TaskParameters	parameters;
	try {
		info = _tasks->info(taskid);
		parameters = _tasks->parameters(taskid);
	} catch (const std::exception& x) {
		return;
	}
	_taskid = taskid;

	// udpate parameters
	ui->instrumentField->setText(QString(parameters.instrument.c_str()));
	if (parameters.cameraIndex >= 0) {
		ui->cameraField->setText(
			QString::number(parameters.cameraIndex));
	} else {
		ui->cameraField->setText(QString());
	}
	if (parameters.ccdIndex >= 0) {
		ui->ccdField->setText(
			QString::number(parameters.ccdIndex));
	} else {
		ui->ccdField->setText(QString());
	}
	if (parameters.coolerIndex >= 0) {
		ui->coolerField->setText(
			QString::number(parameters.coolerIndex));
		ui->temperatureField->setText(QString(astro::stringprintf("%.1f°C",
			parameters.ccdtemperature - 273.15).c_str()));
	} else {
		ui->coolerField->setText(QString());
		ui->temperatureField->setText(QString());
	}
	if (parameters.filterwheelIndex >= 0) {
		ui->filterwheelField->setText(
			QString::number(parameters.filterwheelIndex));
		ui->filterField->setText(QString(parameters.filter.c_str()));
	} else {
		ui->filterwheelField->setText(QString());
		ui->filterField->setText(QString());
	}
	if (parameters.mountIndex >= 0) {
		ui->mountField->setText(QString::number(parameters.mountIndex));
	} else {
		ui->mountField->setText(QString());
	}
	ui->projectField->setText(QString(parameters.project.c_str()));
	ui->repositoryField->setText(QString(parameters.repository.c_str()));

	// update info
	ui->taskidField->setText(QString::number(taskid));
	ui->stateField->setText(QString(
				snowstar::state2string(info.state).c_str()));
	time_t  when = snowstar::converttime(info.lastchange);
	struct tm       *tmp = localtime(&when);
	char    buffer[100];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	ui->lastchangeField->setText(QString(buffer));
	ui->imagerectangleField->setText(QString(
		astro::stringprintf("%dx%d@(%d,%d)",
			info.frame.size.width, info.frame.size.height,
			info.frame.origin.x, info.frame.origin.y).c_str()));
	if (info.state == snowstar::TskCOMPLETE) {
		ui->filenameLabel->setText(QString("Filename:"));
		ui->filenameField->setText(QString(info.filename.c_str()));
	} else {
		ui->filenameLabel->setText(QString("Cause:"));
		ui->filenameField->setText(QString(info.cause.c_str()));
	}
	ui->cameraurlField->setText(QString(info.camera.c_str()));
	ui->ccdurlField->setText(QString(info.ccd.c_str()));
	ui->coolerurlField->setText(QString(info.cooler.c_str()));
	ui->filterwheelurlField->setText(QString(info.filterwheel.c_str()));
	ui->mounturlField->setText(QString(info.mount.c_str()));

	// update title
	setWindowTitle(QString(astro::stringprintf("Info for Task %d",
		taskid).c_str()));
}

void	taskinfowidget::refreshClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "refreshClicked()");
}

void	taskinfowidget::imageClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imageClicked()");
}

void	taskinfowidget::closeClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "closeClicked()");
	close();
}

void	taskinfowidget::taskUpdate(snowstar::TaskMonitorInfo info) {
	if (_taskid != info.taskid) {
		return;
	}
	updateTask(_taskid);
}

void	taskinfowidget::closeEvent(QCloseEvent *) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "close event");
	emit completed();
	deleteLater();
}

} // namespace snowgui
