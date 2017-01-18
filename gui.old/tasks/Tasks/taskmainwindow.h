/*
 * taskmainwindow.h -- define TaskMainWindow
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef TASKMAINWINDOW_H
#define TASKMAINWINDOW_H

#include <QMainWindow>
#include <tasks.hh>
#include <QTimer>
#include <deque>
#include <pthread.h>
#include <set>
#include <downloadparameters.h>

namespace Ui {
class TaskMainWindow;
}

/**
 * \brief Namespace for the task monitor callback servant
 *
 * The server can perform callbacks to inform the client about status updates
 * of tasks. We place all classes that we need for these callbacks in a
 * separate namespace. The reason for this is that we are using somewhat
 * generic names for the callbacks, which may lead to name collisions if
 * we don't create a separate namespace for them.
 */
namespace taskmonitor {

class TaskMonitor_impl;

} // namespace taskmonitor

/**
 * \brief Main window of the Task Manater
 *
 * The main window of the task manager essentially contains a list of tasks
 * in different states. In addition, there is a widget that can be used to
 * create new tasks and submit them to the task queue.
 */
class TaskMainWindow : public QMainWindow {
	Q_OBJECT

	/**
	 * \brief Timer for periodic stuff
	 *
	 * The Task monitor queries the status of the task queue at regular
	 * intervals, e.g. it queries the task queue state every second.
	 * Anything that must be done regularly should be implemented in
	 * the tick() slot.
	 */
	QTimer	*timer;

	/**
	 * \brief Reference to the task queue server object
	 *
	 * This object is used whenever a call to the server is needed
	 */
	Astro::TaskQueue_var	taskqueue;

	// variables and methods related to the task list
	void	retrieveTasklist();

	void	addTasks(const std::set<long>& taskids);

	/**
	 * \brief Callback for task queue monitoring
	 */
	taskmonitor::TaskMonitor_impl	*tm_impl;
	int	monitorid;

	int	indexForTask(int taskid);
	
	void	remove(int taskid);

	/**
	 * \brief task id queue to process
	 *
	 * When a callback is received, the task id is registered in the
	 * this queue. This reduces the amount of work that needs to be
	 * done in the callback, and also removes some concurrency issues.
	 * Since all work is always done in the main thread, only the
	 * taskid queue must be protected against concurrent access. All the
	 * other data structures of this class are only accessed from the
	 * main thread.
	 */
	std::deque<int>	taskids;
	pthread_mutex_t	lock;	// protect the ilst of taskids

	// retrieve a list of selected task ids
	std::list<long>	selectedTaskids();

	/**
	 * \brief parameters for file naming during download
	 *
	 * When downloading image files, new names have to be assigned to
	 * files, as the file names on the server side are completely
	 * random. The parameters for this naming process are kept in
	 * downloadparameters. This object is shared throughtout the
	 * program, so that whenever a download is requested, the parameters
	 * from last time are used as defaults. This gives reasonable
	 * values because it is impossible for the file names generated for
	 * different tasks ever to collide.
	 */
	DownloadParameters	downloadparameters;
public:
	explicit TaskMainWindow(QWidget *parent = 0);
	~TaskMainWindow();

signals:
	void	submitTask(int multiplicity);

private slots:
	void	tick();
	void	taskRealUpdate(int taskid);

public slots:
	void	startQueue();
	void	stopQueue();
	void	downloadSelected();
	void	deleteSelected();
	void	handleToolbarAction(QAction *);
	void	submitTask();
	void	taskUpdateSlot(int taskid);
	void	selectionChanged();
	void	fileSelected(const QString& file);
	void	downloadParametersAccepted();

private:
	Ui::TaskMainWindow *ui;
};

namespace taskmonitor {

/**
 * \brief Task Monitor for the TaskMainWindow class
 *
 * This servant receives task queue updates from the server. Whenever a
 * task changes state on the server, an update is received by the
 * update method.
 */
class TaskMonitor_impl : public POA_Astro::TaskMonitor {
	TaskMainWindow&	_mainwindow;
public:
	TaskMonitor_impl(TaskMainWindow& mainwindow);
	virtual ~TaskMonitor_impl();
	virtual void	update(const Astro::TaskMonitorInfo& tmi);
	virtual void	stop();
};

} // namespace taskmonitor

#endif // TASKMAINWINDOW_H
