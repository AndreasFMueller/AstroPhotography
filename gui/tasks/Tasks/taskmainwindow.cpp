#include "taskmainwindow.h"
#include "ui_taskmainwindow.h"
#include <connectiondialog.h>
#include <stdexcept>
#include <AstroDebug.h>

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

	// retrieve the task lists
	retrieveTasklist();

	// other initializations
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
	timer->start(1000);
}

TaskMainWindow::~TaskMainWindow()
{
    delete ui;
}

void	TaskMainWindow::addTasks(Astro::TaskQueue::taskidsequence_var taskids) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %d tasks", taskids->length());
	for (int i = 0; i < taskids->length(); i++) {
		int	taskid = taskids[i];
		try {
			Astro::TaskInfo_var	info = taskqueue->info(taskid);
			taskinfo.insert(std::make_pair(taskid, info));
			Astro::TaskParameters_var	params
				= taskqueue->parameters(taskid);
			taskparameters.insert(std::make_pair(taskid, params));
		} catch (const Astro::NotFound) {
			debug(LOG_ERR, DEBUG_LOG, 0, "task %d not found",
			taskid);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot get info for task %d", taskid);
		}
	}
}

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

void	TaskMainWindow::tick() {
	Astro::TaskQueue::QueueState	state = taskqueue->state();
	
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

void	TaskMainWindow::startQueue() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start queue");
	taskqueue->start();
}

void	TaskMainWindow::stopQueue() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop queue");
	taskqueue->stop();
}

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
