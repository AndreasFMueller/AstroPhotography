/*
 * taskmainwindow.cpp -- TaskMainWindow implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "taskmainwindow.h"
#include "ui_taskmainwindow.h"
#include <connectiondialog.h>
#include <stdexcept>
#include <AstroDebug.h>
#include <TaskItem.h>
#include <cassert>

/**
 * \brief Create a new TaskMainWindow
 */
TaskMainWindow::TaskMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TaskMainWindow)
{
    ui->setupUi(this);

	// connect to the task manager
	CosNaming::Name	name;
	name.length(2);
	name[0].id = "Astro";
	name[0].kind = "context";
	name[1].id = "TaskQueue";
	name[1].kind = "object";

	// resolve the name
	CORBA::Object_var	obj
		= ConnectionDialog::namingcontext->resolve(name);
	taskqueue = Astro::TaskQueue::_narrow(obj);
	if (CORBA::is_nil(taskqueue)) {
		// XXX should display a dialog explaining the problem
		throw std::runtime_error("nil object reference");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a task queue");
	ui->creatorWidget->taskqueue(taskqueue);

	// retrieve the task lists
	retrieveTasklist();

	// other initializations
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
	timer->start(1000);

	// connect the submitTask signal with the task creators submit slot
	connect(this, SIGNAL(submitTask(int)),
		ui->creatorWidget, SLOT(submitTask(int)),
		Qt::QueuedConnection);
	connect(this, SIGNAL(taskUpdateSignal(int)),
		this, SLOT(taskRealUpdate(int)),
		Qt::QueuedConnection);

	// to register the callback, we need the POA
	obj = ConnectionDialog::orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
	assert(!CORBA::is_nil(root_poa));

	// create the monitor
	tm_impl = new taskmonitor::TaskMonitor_impl(*this);
	PortableServer::ObjectId_var	tmid
		= root_poa->activate_object(tm_impl);
	tm_impl->_remove_ref();

	// get a reference to register
	CORBA::Object_var       tmobj
		= root_poa->id_to_reference(tmid);
	Astro::TaskMonitor_var      tmvar
		= Astro::TaskMonitor::_narrow(tmobj);

	// register the callback with the task queue
	monitorid = taskqueue->registerMonitor(tmvar);
}

/**
 * \brief Destroy the TaskMainWindow
 */
TaskMainWindow::~TaskMainWindow()
{
	taskqueue->unregisterMonitor(monitorid);
    delete ui;
}

/**
 * \brief Add the tasks from a sequence of ids
 */
void	TaskMainWindow::addTasks(Astro::TaskQueue::taskidsequence_var taskids) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %d tasks", taskids->length());
	time_t	now = time(NULL);
	for (int i = 0; i < taskids->length(); i++) {
		int	taskid = taskids[i];
		try {
			// get the task info
			Astro::TaskInfo_var	info = taskqueue->info(taskid);
			info->lastchange = now - info->lastchange;
			taskinfo.insert(std::make_pair(taskid, info));
			Astro::TaskParameters_var	params
				= taskqueue->parameters(taskid);
			taskparameters.insert(std::make_pair(taskid, params));

			// create a list item
			QListWidgetItem	*lwi = new QListWidgetItem();
			lwi->setSizeHint(QSize(300,90));
			TaskItem	*ti = new TaskItem(info, params);
			ui->tasklistWidget->addItem(lwi);
			ui->tasklistWidget->setItemWidget(lwi, ti);
		} catch (const Astro::NotFound) {
			debug(LOG_ERR, DEBUG_LOG, 0, "task %d not found",
			taskid);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot get info for task %d", taskid);
		}
	}
}

/**
 * \brief Retrieve the task list
 */
void	TaskMainWindow::retrieveTasklist() {
	// remove all task information from the task list
	taskinfo.clear();
	taskparameters.clear();

	// process each state in sequence
	Astro::TaskQueue::taskidsequence_var	pending
		= taskqueue->tasklist(Astro::TASK_PENDING);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pending: %d", pending->length());
	addTasks(pending);

	Astro::TaskQueue::taskidsequence_var	executing
		= taskqueue->tasklist(Astro::TASK_EXECUTING);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "executing: %d", executing->length());
	addTasks(executing);

	Astro::TaskQueue::taskidsequence_var	failed
		= taskqueue->tasklist(Astro::TASK_FAILED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "failed: %d", failed->length());
	addTasks(failed);

	Astro::TaskQueue::taskidsequence_var	cancelled
		= taskqueue->tasklist(Astro::TASK_CANCELLED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelled: %d", cancelled->length());
	addTasks(cancelled);

	Astro::TaskQueue::taskidsequence_var	completed
		= taskqueue->tasklist(Astro::TASK_COMPLETED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "completed: %d", completed->length());
	addTasks(completed);
}

/**
 * \brief Task to use every second when the timer fires
 */
void	TaskMainWindow::tick() {
	Astro::TaskQueue::QueueState	state;
	try {
		state = taskqueue->state();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state retrieval failed: %s",
			x.what());
		return;
	}
	
	QString	wt = ConnectionDialog::servername;
	wt.append(" ");
	bool	canstop = false;
	bool	canstart = false;
	switch (state) {
	case Astro::TaskQueue::IDLE:
		wt.append("[idle]");
		canstart = true;
		break;
	case Astro::TaskQueue::LAUNCHING:
		wt.append("[launching]");
		canstop = true;
		break;
	case Astro::TaskQueue::STOPPING:
		wt.append("[stopping]");
		break;
	case Astro::TaskQueue::STOPPED:
		wt.append("[stopped]");
		canstart = true;
		break;
	}
	ui->actionStartQueue->setEnabled(canstart);
	ui->actionStopQueue->setEnabled(canstop);
	if (wt != windowTitle()) {
		setWindowTitle(wt);
	}
}

/**
 * \brief slot that starts the task queue
 */
void	TaskMainWindow::startQueue() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start queue");
	taskqueue->start();
}

/**
 * \brief Slot that stops the task queue
 */
void	TaskMainWindow::stopQueue() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop queue");
	taskqueue->stop();
}

/**
 * \brief Slot to start tasks in the queue
 */
void	TaskMainWindow::submitTask() {
	int	multiplicity = ui->multiplicitySpinBox->value();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit %d copies of task",
		multiplicity);
	emit submitTask(multiplicity);
}

/**
 * \brief Slot to handle toolbar actions
 */
void	TaskMainWindow::handleToolbarAction(QAction *action) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "toolbar action called");
	if (action == ui->actionStartQueue) {
		startQueue();
		return;
	}
	if (action == ui->actionStopQueue) {
		stopQueue();
		return;
	}
}

/**
 * \brief Update slot
 */
void	TaskMainWindow::taskUpdateSlot(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskUpdateSignal(%d)", taskid);
	emit taskUpdateSignal(taskid);
}

void	TaskMainWindow::taskRealUpdate(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "real task update %d", taskid);

	// retrieve the task information from the 
	Astro::TaskInfo_var	info;
	try {
		info = taskqueue->info(taskid);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get task info: %s",
			x.what());
	}
	time_t	now = time(NULL);
	info->lastchange = now - info->lastchange;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got updated info for task %d", taskid);

	Astro::TaskParameters_var	params
		= taskqueue->parameters(taskid);

	// depending on whether the task already exists in the list,
	// we will add or udpate the stuff
	if (taskinfo.find(taskid) == taskinfo.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "must insert entry");

		// insert entry into maps
		taskinfo.insert(std::make_pair(taskid, info));
		taskparameters.insert(std::make_pair(taskid, params));

		// create a list item
		QListWidgetItem	*lwi = new QListWidgetItem();
		lwi->setSizeHint(QSize(300,90));
		TaskItem	*ti = new TaskItem(info, params);
		ui->tasklistWidget->addItem(lwi);
		ui->tasklistWidget->setItemWidget(lwi, ti);

		// make sure the list is repainted
		repaint();

		// insert case done
		debug(LOG_DEBUG, DEBUG_LOG, 0, "entry for task %d inserted",
			taskid);
		return;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "we have to update task %d", taskid);

	// udpate the info
	taskinfo.find(taskid)->second = info;

	// find the item in the list that matches the id
	for (int i = 0; i < ui->tasklistWidget->count(); i++) {
		QListWidgetItem	*lwi = ui->tasklistWidget->item(i);
		TaskItem	*ti = (TaskItem *)ui->tasklistWidget->itemWidget(lwi);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "entry %d: taskid = %d",
			i, ti->id());
		if (ti->id() != taskid) {
			continue;
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "update entry %d", i);
			ti->updateInfo(info);
			ti->repaint();
		}
	}

	repaint();
}

namespace taskmonitor {

TaskMonitor_impl::TaskMonitor_impl(TaskMainWindow& mainwindow)
	: _mainwindow(mainwindow) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Task monitor created");
}

TaskMonitor_impl::~TaskMonitor_impl() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn of task monitoring");
}

void	TaskMonitor_impl::update(const Astro::TaskMonitorInfo& taskinfo) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got update for task %d, new state %d",
		taskinfo.taskid, taskinfo.newstate);
	_mainwindow.taskUpdateSlot(taskinfo.taskid);
}

void	TaskMonitor_impl::stop() {
}

} // namespace taskmonitor
