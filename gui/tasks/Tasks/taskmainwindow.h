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

namespace taskmonitor {

class TaskMonitor_impl;

} // namespace taskmonitor

class TaskMainWindow : public QMainWindow
{
	Q_OBJECT
	QTimer	*timer;
	Astro::TaskQueue_var	taskqueue;
	void	retrieveTasklist();
	typedef std::map<int, Astro::TaskInfo_var>	taskinfo_t;
	typedef std::map<int, Astro::TaskParameters_var>	taskparameter_t;
	taskinfo_t	taskinfo;
	taskparameter_t	taskparameters;
	void	addTasks(const std::set<long>& taskids);
	taskmonitor::TaskMonitor_impl	*tm_impl;
	int	monitorid;
	void	remove(int taskid);
	std::deque<int>	taskids;
	pthread_mutex_t	lock;
	std::list<long>	selectedTaskids();
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
	void	buttonSlot(int taskid);
	void	selectionChanged();
	void	fileSelected(const QString& file);
	void	downloadParametersAccepted();

private:
	Ui::TaskMainWindow *ui;
};

namespace taskmonitor {

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
