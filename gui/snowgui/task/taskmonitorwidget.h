/*
 * taskmonitorwidget.h -- widget to monitor task state changes
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TASKMONITORWIDGET_H
#define SNOWGUI_TASKMONITORWIDGET_H

#include <QWidget>
#include "TaskMonitorController.h"
#include <tasks.h>
#include <AstroDiscovery.h>

namespace snowgui {

namespace Ui {
	class taskmonitorwidget;
}

class taskmonitorwidget : public QWidget {
	Q_OBJECT
	snowstar::TaskQueuePrx	_tasks;
	TaskMonitorController*	_taskmonitor;
	Ice::ObjectPtr		_taskmonitorptr;
public:
	explicit taskmonitorwidget(QWidget *parent = 0);
	~taskmonitorwidget();

	virtual void	setServiceObject(
		astro::discover::ServiceObject serviceobject);

private:
	Ui::taskmonitorwidget *ui;
public slots:
	void	taskUpdate(snowstar::TaskMonitorInfo);
};

} // namespace snowgui

#endif // SNOWGUI_TASKMONITORWIDGET_H
