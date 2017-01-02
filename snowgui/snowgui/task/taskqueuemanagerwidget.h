/*
 * taskqueuemanagerwidget.h -- manage a task queue
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TASKQUEUEMANAGERWIDGET_H
#define SNOWGUI_TASKQUEUEMANAGERWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <tasks.h>
#include <repository.h>
#include <AstroDiscovery.h>
#include "TaskMonitorController.h"

namespace snowgui {

namespace Ui {
	class taskqueuemanagerwidget;
}

class taskqueuemanagerwidget : public QWidget {
	Q_OBJECT

	snowstar::TaskQueuePrx		_tasks;
	snowstar::RepositoriesPrx	_repositories;
	snowstar::ImagesPrx		_images;
	TaskMonitorController*	_taskmonitor;
	Ice::ObjectPtr	_taskmonitorptr;

public:
	explicit taskqueuemanagerwidget(QWidget *parent = 0);
	~taskqueuemanagerwidget();
	void	setServiceObject(astro::discover::ServiceObject serviceobject);

signals:
	void	imageReceived(astro::image::ImagePtr);

public slots:
	void	infoClicked();
	void	cancelClicked();
	void	imageClicked();
	void	downloadClicked();
	void	deleteClicked();
	void	itemSelectionChanged();
	void	taskUpdate(snowstar::TaskMonitorInfo);


private:
	Ui::taskqueuemanagerwidget *ui;

	
	void    addTasks(QTreeWidgetItem *parent,
			snowstar::TaskState state);
	void    addTasks();
	void	addTask(int taskid);
	void	addTask(QTreeWidgetItem *parent, const snowstar::TaskInfo&,
			const snowstar::TaskParameters&);
	void	setHeader(snowstar::TaskState state);
	void	setHeaders();
	QTreeWidgetItem	*parent(snowstar::TaskState state);
	void	deleteTask(int taskid);
	void	updateInfo(QTreeWidgetItem *, const snowstar::TaskInfo&);
};

} // namespace snowgui

#endif // SNOWGUI_TASKQUEUEMANAGERWIDGET_H
