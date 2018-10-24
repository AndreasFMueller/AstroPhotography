/*
 * taskstatuswidget.cpp -- display current task status, implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "taskstatuswidget.h"
#include "ui_taskstatuswidget.h"
#include <CommunicatorSingleton.h>

namespace snowgui {

/**
 * \brief Create a status widget
 */
taskstatuswidget::taskstatuswidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::taskstatuswidget) {
	ui->setupUi(this);

	connect(ui->startstopButton, SIGNAL(clicked()),
		this, SLOT(startClicked()));
	_statusTimer.setInterval(100);
	connect(&_statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));

	// this weird setup is necessary to work around the problem that
	// the timer can only be started from the main thread
	connect(this, SIGNAL(started()), this, SLOT(dostart()));
	connect(this, SIGNAL(updateSignal(snowstar::QueueState)),
		ui->taskstateWidget, SLOT(update(snowstar::QueueState)));
}

/**
 * \brief destroy the status widget
 */
taskstatuswidget::~taskstatuswidget() {
	_statusTimer.stop();
	delete ui;
}

/**
 * \brief ICE initialisations
 *
 * This is needed for the widget to get a tasks proxy to query the 
 * the task queue on the server.
 */
void	taskstatuswidget::setServiceObject(
		astro::discover::ServiceObject serviceobject) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up the service object");

	// get the Tasks proxy
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
		serviceobject.connect("Tasks"));
	_tasks = snowstar::TaskQueuePrx::checkedCast(base);
	if (!_tasks) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get a taskqueue");
	}

	// get the status
	snowstar::QueueState	currentstate = _tasks->state();
	emit updateSignal(currentstate);
	update(currentstate);

	// start the timer so that we will get updates at regular intervals
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting the status timer");
	emit started();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setup service object complete");
}

/**
 * \brief Slot to start the timer
 *
 * This is needed because the setServiceObject may come from a different
 * thread and may not start the timer, as the timer has to be started
 * from the main thread.
 */
void	taskstatuswidget::dostart() {
	_statusTimer.start();
}

/**
 * \brief Slot called when the start/stop botton is clicked
 */
void	taskstatuswidget::startClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startClicked()");
	if (!_tasks) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "we have not tasks proxy");
		return;
	}
	snowstar::QueueState	currentstate = _tasks->state();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current state: %d", (int)currentstate);
	switch (currentstate) {
	case snowstar::QueueIDLE:
	case snowstar::QueueSTOPPED:
		// in the idle and stopped state, we can get the queue to
		// launch new jobs
		_tasks->start();
		break;
	case snowstar::QueueLAUNCHING:
		// stop launching new 
		_tasks->stop();
		break;
	case snowstar::QueueSTOPPING:
		// we cannot do anything while the queue is stopping
		break;
	}
	// if the state changes, the timer will pick it up
}

/**
 * \brief Update the state
 *
 * This slot is called when the statusUpdate slot decides that it is
 * necessary to update the state
 */
void	taskstatuswidget::update(snowstar::QueueState state) {
	_state = state;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update to new state: %d", (int)_state);
	emit updateSignal(_state);

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

/**
 * \brief This slot is called by the timer to query the state
 *
 * Only if we detect a state change on the server will wi update
 * the state on the client
 */
void	taskstatuswidget::statusUpdate() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "statusUpdate()");
	if (!_tasks) {
		return;
	}
	snowstar::QueueState	newstate = _tasks->state();
	if (_state != newstate) {
		update(newstate);
	}
}

} // namespace snowgui
