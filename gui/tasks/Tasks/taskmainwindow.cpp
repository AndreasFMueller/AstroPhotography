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
#include <QFileDialog>
#include <QMessageBox>
#include <downloaddialog.h>
#include <iostream>

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

	// create the mutex
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&lock, &mattr);

	// other initializations
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
	timer->start(1000);

	// connect the submitTask signal with the task creators submit slot
	connect(this, SIGNAL(submitTask(int)),
		ui->creatorWidget, SLOT(submitTask(int)),
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
void	TaskMainWindow::addTasks(const std::set<long>& taskids) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %d tasks", taskids.size());
	time_t	now = time(NULL);
	std::set<long>::const_iterator	i;
	for (i = taskids.begin(); i != taskids.end(); i++) {
		int	taskid = *i;
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

static void	addids(std::set<long>& taskids,
			Astro::TaskQueue::taskidsequence_var ids) {
	for (int i = 0; i < ids->length(); i++) {
		taskids.insert(ids[i]);
	}
}

/**
 * \brief Retrieve the task list
 */
void	TaskMainWindow::retrieveTasklist() {
	// remove all task information from the task list
	taskinfo.clear();
	taskparameters.clear();

	std::set<long>	taskids;

	// process each state in sequence
	Astro::TaskQueue::taskidsequence_var	pending
		= taskqueue->tasklist(Astro::TASK_PENDING);
	addids(taskids, pending);

	Astro::TaskQueue::taskidsequence_var	executing
		= taskqueue->tasklist(Astro::TASK_EXECUTING);
	addids(taskids, executing);

	Astro::TaskQueue::taskidsequence_var	failed
		= taskqueue->tasklist(Astro::TASK_FAILED);
	addids(taskids, failed);

	Astro::TaskQueue::taskidsequence_var	cancelled
		= taskqueue->tasklist(Astro::TASK_CANCELLED);
	addids(taskids, cancelled);

	Astro::TaskQueue::taskidsequence_var	completed
		= taskqueue->tasklist(Astro::TASK_COMPLETED);
	addids(taskids, completed);

	addTasks(taskids);
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

	// process all the task ids in the queue
	pthread_mutex_lock(&lock);
	while (taskids.size()) {
		int	taskid = taskids.front();
		taskids.pop_front();
		pthread_mutex_unlock(&lock);

		try {
			taskRealUpdate(taskid);
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "error in taskid %d",
				taskid);
		}

		pthread_mutex_lock(&lock);
	}
	pthread_mutex_unlock(&lock);
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
 * \brief Retrieve a list of selected task ids
 */
std::list<long>	TaskMainWindow::selectedTaskids() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve a list of selected items");
	std::list<long>	taskids;
	for (int i = 0; i < ui->tasklistWidget->count(); i++) {
		QListWidgetItem	*item = ui->tasklistWidget->item(i);
		if (item->isSelected()) {
			TaskItem	*ti = (TaskItem *)ui->tasklistWidget->itemWidget(item);
			taskids.push_back(ti->id());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d selected task ids", taskids.size());
	return taskids;
}

/**
 * \brief Download all selected 
 */
void	TaskMainWindow::downloadSelected() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download selected entries");
	std::list<long>	selected = selectedTaskids();
	if (selected.size() == 0) {
		return;
	}

	// create a parameter dialog
	DownloadDialog	*dialog = new DownloadDialog(downloadparameters, this);
	dialog->setModal(true);
	dialog->show();
}

/**
 * \brief Slot called when the download parameters are accepted
 */
void	TaskMainWindow::downloadParametersAccepted() {
	// open a file dialog to select the directory where the
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a file dialog");
	QFileDialog	*filedialog = new QFileDialog(this);
	//filedialog->setAcceptMode(QFileDialog::AcceptSave);
	filedialog->setFileMode(QFileDialog::DirectoryOnly);
	filedialog->show();
	connect(filedialog, SIGNAL(fileSelected(const QString&)),
		this, SLOT(fileSelected(const QString&)),
		Qt::QueuedConnection);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "dialog created");
}

/**
 * \brief Slot called when a directory is selected
 */
void	TaskMainWindow::fileSelected(const QString& directory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file selected");
	downloadparameters.directory = directory;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "directory string length %d",
		downloadparameters.directory.length());

	// perform the download of all files
	downloadparameters.download(taskqueue, selectedTaskids());
}

/**
 * \brief Delete selected items
 */
void	TaskMainWindow::deleteSelected() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete selected entries");
	std::list<long>	selected = selectedTaskids();

	QMessageBox	msgBox;
	msgBox.setText("Delete task");
	char	buffer[128];
	snprintf(buffer, sizeof(buffer),
		"Do you really want to delete %d tasks?",
		selected.size());
	msgBox.setInformativeText(buffer);
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Ok);

	int	ret = msgBox.exec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ret = %d", ret);

	switch (ret) {
	case QMessageBox::Ok:
		break;
	case QMessageBox::Cancel:
		return;
	}

	std::list<long>::const_iterator	i;
	for(i = selected.begin(); i != selected.end(); i++) {
		try {
			taskqueue->remove(*i);
		} catch (const std::exception& x) {
		
		} catch (...) {
			
		}
	}
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
	if (action == ui->actionDelete) {
		deleteSelected();
		return;
	}
	if (action == ui->actionDownload) {
		downloadSelected();
		return;
	}
}

/**
 * \brief Update slot
 */
void	TaskMainWindow::taskUpdateSlot(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskUpdateSlot(%d)", taskid);
	pthread_mutex_lock(&lock);
	taskids.push_back(taskid);
	pthread_mutex_unlock(&lock);
}

/**
 * \brief Really do the update
 */
void	TaskMainWindow::taskRealUpdate(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "real task update %d", taskid);

	// retrieve the task information from the 
	Astro::TaskInfo_var	info;
	try {
		info = taskqueue->info(taskid);
	} catch (Astro::NotFound x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d not found, removing it",
			taskid);
		remove(taskid);
		return;
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

void	TaskMainWindow::buttonSlot(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle the button in task %d", taskid);
	try {
		Astro::TaskInfo	info = *taskqueue->info(taskid);
		if (info.state == Astro::TASK_EXECUTING) {
			taskqueue->cancel(taskid);
		} else {
			taskqueue->remove(taskid);
		}
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot cancel/remove: %s",
			x.what());
	}
}

void	TaskMainWindow::remove(int taskid) {
	taskinfo.erase(taskid);
	taskparameters.erase(taskid);

	// find the item in the list that matches the id
	for (int i = 0; i < ui->tasklistWidget->count(); i++) {
		QListWidgetItem	*lwi = ui->tasklistWidget->item(i);
		TaskItem	*ti = (TaskItem *)ui->tasklistWidget->itemWidget(lwi);
		if (ti->id() == taskid) {
			QListWidgetItem	*lwi = ui->tasklistWidget->takeItem(i);
			delete lwi;
			return;
		}
	}
}

void	TaskMainWindow::selectionChanged() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "selection changed");
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
