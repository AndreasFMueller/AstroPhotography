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
#include <downloadreportdialog.h>

/**
 * \brief Create a new TaskMainWindow
 */
TaskMainWindow::TaskMainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::TaskMainWindow)
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
//	tm_impl->_remove_ref();

	// get a reference to register
	CORBA::Object_ptr       tmobj
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
 *
 * Any tasks not available from the server are simply ignored
 */
void	TaskMainWindow::addTasks(const std::set<long>& taskids) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %d tasks", taskids.size());
	time_t	now = time(NULL); // needed to recompute timestamps

	std::set<long>::const_iterator	i;
	for (i = taskids.begin(); i != taskids.end(); i++) {
		int	taskid = *i;
		try {
			// get the task info
			Astro::TaskInfo_var	info = taskqueue->info(taskid);
			info->lastchange = now - info->lastchange;

			// get parameters
			Astro::TaskParameters_var	params
				= taskqueue->parameters(taskid);

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
 * \brief Auxiliary function to add ids 
 *
 * The server returns task ids in a taskidsequence_var, which is much
 * less nice to work with than the standard template library containers.
 * This method adds the ids contained into a sorted STL container, which
 * solves the problem that the task list should be sorted.
 */
static void	addids(std::set<long>& taskids,
			Astro::TaskQueue_var& taskqueue,
			const Astro::TaskState& state) {
	Astro::TaskQueue::taskidsequence_var	ids
		= taskqueue->tasklist(state);
	for (int i = 0; i < ids->length(); i++) {
		taskids.insert(ids[i]);
	}
}

/**
 * \brief Retrieve the task list
 */
void	TaskMainWindow::retrieveTasklist() {
	std::set<long>	taskids;

	addids(taskids, taskqueue, Astro::TASK_PENDING);
	addids(taskids, taskqueue, Astro::TASK_EXECUTING);
	addids(taskids, taskqueue, Astro::TASK_FAILED);
	addids(taskids, taskqueue, Astro::TASK_CANCELLED);
	addids(taskids, taskqueue, Astro::TASK_COMPLETED);

	// now add all tasks in the set to the task list
	addTasks(taskids);
}

/**
 * \brief Task to use every second when the timer fires
 *
 * This method should contain everything that needs to be done periodically. 
 * In particular, it should handle status updates for the task queue, as we
 * are not handling these in the callback (the callback can only inform us of
 * stop, not of start). This method also processes all task ids in the
 * queue of received updates.
 */
void	TaskMainWindow::tick() {
	// query queue state
	Astro::TaskQueue::QueueState	state;
	try {
		state = taskqueue->state();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "state retrieval failed: %s",
			x.what());
		return;
	}
	
	// build new window title and find out which actions to enable
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

	// enable/disable actions
	ui->actionStartQueue->setEnabled(canstart);
	ui->actionStopQueue->setEnabled(canstop);

	// set window title
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
 *
 * This method scans the task list and constructs a list of tasks that
 * are selected.
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
 * \brief Download all selected task
 *
 * This Method checks whether anything is selected and if so, it opens
 * the dialog where the user can enter parameters by which file names
 * will be created.
 */
void	TaskMainWindow::downloadSelected() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "downloadSelected() slot called");
	std::list<long>	selected = selectedTaskids();
	if (selected.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "nothing to download");
		return;
	}

	// create a parameter dialog
	DownloadDialog	*dialog = new DownloadDialog(downloadparameters, this);
	dialog->setModal(true);
	dialog->show();
}

/**
 * \brief Slot called when the download parameters are accepted
 *
 * When the download dialog is accepted, we still need some more information,
 * namely the name of a directory where the files can be saved. This is
 * done in a standard QFileDialog opened from this slot.
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
 *
 * When the directory for the download is accepted, this slot is called
 * to actually perform the download for all files.
 */
void	TaskMainWindow::fileSelected(const QString& directory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file selected");
	downloadparameters.directory = directory;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "directory string length %d",
		downloadparameters.directory.length());

	// perform the download of all files
	std::list<fileinfo>	files
		= downloadparameters.download(taskqueue, selectedTaskids());

	// create a report widget for the files downloaded
	DownloadReportDialog	*report
		= new DownloadReportDialog(files, this);
	report->show();
}

/**
 * \brief Delete selected items
 *
 * Before deleting tasks, the user is prompted for a confirmation.
 * If the user accepts, all selected tasks are deleted from the server.
 * The server will then inform the client(s) via the callback mechanism
 * about the changes in the task list.
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

	// ok, we got a configuration, now remove all the tasks
	std::list<long>::const_iterator	i;
	for(i = selected.begin(); i != selected.end(); i++) {
		// remove the task, just ignore any errors
		try {
			taskqueue->remove(*i);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"error while removing task %d: %s",
				*i, x.what());
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"unknown error during remove of %d", *i);
		}
	}
}

/**
 * \brief Slot to start tasks in the queue
 *
 * Slot called when the submit button is called. This slot reads the
 * multiplicity of the task, and emits a signal with the multiplicity
 * argument. The constructor has connected this signal to the task creator's
 * slot with the same signator, the TaskCreator then submits all the new
 * tasks.
 */
void	TaskMainWindow::submitTask() {
	int	multiplicity = ui->multiplicitySpinBox->value();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit %d copies of task",
		multiplicity);
	emit submitTask(multiplicity);
}

/**
 * \brief Slot to handle toolbar actions
 *
 * This is just a dispatcher method that calls the appropriate acction 
 * handler slot.
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
 * \brief Find the index of the entry with the matching task id
 */
int	TaskMainWindow::indexForTask(int taskid) {
	for (int i = 0; i < ui->tasklistWidget->count(); i++) {
		QListWidgetItem	*lwi = ui->tasklistWidget->item(i);
		TaskItem	*ti = (TaskItem *)ui->tasklistWidget->itemWidget(lwi);
		if (ti->id() == taskid) {
			return i;
		}
	}
	return -1;
}


/**
 * \brief Update slot
 *
 * This slot is called by the task monitor. It just adds the task id reported
 * by the monitor to the queue of task ids to udpate.
 */
void	TaskMainWindow::taskUpdateSlot(int taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskUpdateSlot(%d)", taskid);
	pthread_mutex_lock(&lock);
	taskids.push_back(taskid);
	pthread_mutex_unlock(&lock);
}

/**
 * \brief Really do the update
 *
 * When the timer's action slot finds out that some task ids have changed,
 * this slot is called to actually perform the udpate
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
		return;
	} catch (const CORBA::SystemException& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "corba system exception");
		return;
	} catch (const CORBA::UserException& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "corba user exception");
		return;
	} catch (const CORBA::Exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "corba exception");
		return;
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown error");
		throw;
		return;
	}
	time_t	now = time(NULL);
	info->lastchange = now - info->lastchange;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got updated info for task %d", taskid);

	// depending on whether the task already exists in the list,
	// we will add or udpate the stuff
	int	index = indexForTask(taskid);
	if (index < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "must insert entry");

		// to insert an entry, we also need the parameters
		Astro::TaskParameters_var	params
			= taskqueue->parameters(taskid);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"task parameters for %d retrieved", taskid);

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

	// retrieve the task TaskItem from the list, 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "we have to update task %d", taskid);
	QListWidgetItem	*lwi = ui->tasklistWidget->item(index);
	TaskItem	*ti = (TaskItem *)ui->tasklistWidget->itemWidget(lwi);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update entry %d", index);
	ti->updateInfo(info);

	// we have to repaint the item, because otherwise it will only repaint
	// when the it becomes visible after a list move, or when the window
	// receives a repaint event.
	ti->repaint();
}

/**
 * \brief Remove a task from the tasklist
 *
 * This method deletes task list entries. It is called from the timer tick
 * method and the taskRealUpdate slot.
 */
void	TaskMainWindow::remove(int taskid) {

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

/** 
 * \brief Slot called when selection has changed
 *
 * This can be used to udpate action buttons in the toolbar, as they
 * are only available if any tasks are selected.
 */
void	TaskMainWindow::selectionChanged() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "selection changed");
	std::list<long>	selectedids = TaskMainWindow::selectedTaskids();
	bool	someselected = (selectedids.size() > 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "someselected: %s", (someselected) ? "YES" : "NO");
	ui->actionDownload->setEnabled(someselected);
	ui->actionDelete->setEnabled(someselected);
}

