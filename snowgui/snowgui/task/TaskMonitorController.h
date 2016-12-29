/*
 * TaskMonitorController.h -- controller to monitor tasks
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _TaskMonitorController_h
#define _TaskMonitorController_h

#include <QObject>
#include <tasks.h>

namespace snowgui {

class TaskMonitorController : public QObject, public snowstar::TaskMonitor {
	Q_OBJECT

	snowstar::TaskQueuePrx	_tasks;
protected:
	Ice::Identity	_myidentity;
public:
	const Ice::Identity&	identity() const { return _myidentity; }
	explicit TaskMonitorController(QObject *parent = NULL);
	~TaskMonitorController();

	// configure the monitor
	void	setTasks(snowstar::TaskQueuePrx tasks, Ice::ObjectPtr myself);

	// callback methods
	void	stop(const Ice::Current&);
	void	update(const snowstar::TaskMonitorInfo&, const Ice::Current&);
signals:
	void	taskUpdate(snowstar::TaskMonitorInfo);
};

} // namespace snowgui

#endif /* _TaskMonitorController_h */
