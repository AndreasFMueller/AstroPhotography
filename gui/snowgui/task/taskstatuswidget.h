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

	QTimer	_statusTimer;

public:
	explicit taskstatuswidget(QWidget *parent = 0);
	~taskstatuswidget();

	void	setServiceObject(astro::discover::ServiceObject serviceobject);

	void	update(snowstar::QueueState);

signals:
	void	started();
	void	updateSignal(snowstar::QueueState);

public slots:
	void	startClicked();
	void	statusUpdate();
	void	dostart();

private:
	Ui::taskstatuswidget *ui;
	snowstar::TaskQueuePrx	_tasks;
	snowstar::QueueState	_state;
};

} // namespace snowgui

#endif // SNOWGUI_TASKSTATUSWIDGET_H
