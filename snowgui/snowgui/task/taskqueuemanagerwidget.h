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

namespace snowgui {

namespace Ui {
	class taskqueuemanagerwidget;
}

class taskqueuemanagerwidget : public QWidget {
	Q_OBJECT

	snowstar::TaskQueuePrx		_tasks;
	snowstar::RepositoriesPrx	_repositories;

public:
	explicit taskqueuemanagerwidget(QWidget *parent = 0);
	~taskqueuemanagerwidget();
	void	setServiceObject(astro::discover::ServiceObject serviceobject);

public slots:
	void	infoClicked();
	void	cancelClicked();
	void	imageClicked();
	void	downloadClicked();
	void	deleteClicked();

private:
	Ui::taskqueuemanagerwidget *ui;

	
	void    addTasks(QTreeWidgetItem *parent,
			snowstar::TaskState state);
	void    addTasks();
};

} // namespace snowgui

#endif // SNOWGUI_TASKQUEUEMANAGERWIDGET_H
