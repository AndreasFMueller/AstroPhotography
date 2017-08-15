/*
 * EventMonitor.h -- a monitor class to monitor general events
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _EventMonitor_h
#define _EventMonitor_h

#include <QObject>
#include <types.h>

namespace snowgui {

class EventMonitor : public QObject, public snowstar::EventMonitor {
	Q_OBJECT
	
public:
	EventMonitor();
	virtual void	update(const snowstar::Event& event,
				const Ice::Current& current);
	virtual void	stop(const Ice::Current& current);
signals:
	void	updateSignal(snowstar::Event);
	void	stopSignal();
};

} // namespace snowgui

#endif /* _EventMonitor_h */
