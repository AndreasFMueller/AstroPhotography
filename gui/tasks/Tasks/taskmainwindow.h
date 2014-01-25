#ifndef TASKMAINWINDOW_H
#define TASKMAINWINDOW_H

#include <QMainWindow>
#include <tasks.hh>
#include <QTimer>

namespace Ui {
class TaskMainWindow;
}

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
	void	addTasks(Astro::TaskQueue::taskidsequence_var taskids);
public:
	explicit TaskMainWindow(QWidget *parent = 0);
	~TaskMainWindow();

private slots:
	void	tick();

public slots:
	void	startQueue();
	void	stopQueue();
	void	handleToolbarAction(QAction *);

private:
	Ui::TaskMainWindow *ui;
};

#endif // TASKMAINWINDOW_H
