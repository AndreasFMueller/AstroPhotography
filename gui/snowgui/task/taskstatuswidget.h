/*
 * taskstatuswidget.h -- display to show the current task status
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TASKSTATUSWIDGET_H
#define SNOWGUI_TASKSTATUSWIDGET_H

#include <QWidget>
#include <QTimer>
#include <AstroDiscovery.h>
#include <tasks.h>

namespace snowgui {

namespace Ui {
	class taskstatuswidget;
}

class taskstatuswidget : public QWidget {
	Q_OBJECT

public:
	explicit taskstatuswidget(QWidget *parent = 0);
	~taskstatuswidget();

	void	setServiceObject(astro::discover::ServiceObject serviceobject);

public slots:
	void	startClicked();
	void	update(snowstar::QueueState);
	void	statusUpdate();

private:
	Ui::taskstatuswidget *ui;
	QTimer	statusTimer;
	snowstar::TaskQueuePrx	_tasks;
	snowstar::QueueState	_state;
};

} // namespace snowgui

#endif // SNOWGUI_TASKSTATUSWIDGET_H
