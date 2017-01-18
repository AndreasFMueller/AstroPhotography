/*
 * taskstatuswidget.cpp -- display current task status, implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "taskstatuswidget.h"
#include "ui_taskstatuswidget.h"
#include <CommunicatorSingleton.h>

namespace snowgui {

taskstatuswidget::taskstatuswidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::taskstatuswidget) {
	ui->setupUi(this);

	connect(ui->startstopButton, SIGNAL(clicked()),
		this, SLOT(startClicked()));
	statusTimer.setInterval(100);
	connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
}

taskstatuswidget::~taskstatuswidget() {
	statusTimer.stop();
	delete ui;
}

void	taskstatuswidget::setServiceObject(
		astro::discover::ServiceObject serviceobject) {

	// get the Tasks proxy
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
		serviceobject.connect("Tasks"));
	_tasks = snowstar::TaskQueuePrx::checkedCast(base);
	if (!_tasks) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get a taskqueue");
	}

	// get the status
	ui->taskstateWidget->update(_tasks->state());
	statusTimer.start();
}

void	taskstatuswidget::startClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startClicked()");
	if (!_tasks) {
		return;
	}
	switch (_tasks->state()) {
	case snowstar::QueueIDLE:
		_tasks->start();
		break;
	case snowstar::QueueLAUNCHING:
		_tasks->stop();
		break;
	case snowstar::QueueSTOPPING:
		break;
	case snowstar::QueueSTOPPED:
		_tasks->start();
		break;
	}
}

void	taskstatuswidget::update(snowstar::QueueState state) {
	_state = state;
	ui->taskstateWidget->update(_state);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update to new state");

	switch (_state) {
	case snowstar::QueueIDLE:
		ui->startstopButton->setEnabled(true);
		ui->startstopButton->setText(QString("Start"));
		break;
	case snowstar::QueueLAUNCHING:
		ui->startstopButton->setEnabled(true);
		ui->startstopButton->setText(QString("Stop"));
		break;
	case snowstar::QueueSTOPPING:
		ui->startstopButton->setEnabled(false);
		break;
	case snowstar::QueueSTOPPED:
		ui->startstopButton->setEnabled(true);
		ui->startstopButton->setText(QString("Start"));
		break;
	}
}

void	taskstatuswidget::statusUpdate() {
	snowstar::QueueState	newstate = _tasks->state();
	if (_state != newstate) {
		update(newstate);
	}
}

} // namespace snowgui
