/*
 * HeartbeatMonitor.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _HeartbeatMonitor_h
#define _HeartbeatMonitor_h

#include <QWidget>
#include <QTimer>
#include <types.h>

namespace snowgui {

/**
 * \brief Heartbeat monitoring class
 *
 * This method emits signals if the connection is lost
 */
class HeartbeatMonitor : public QObject, public snowstar::HeartbeatMonitor {
	Q_OBJECT
	int	_multiplier;
	float	_interval;
	QTimer	_timer;
	bool	_lost;
	int	milliseconds() const;
public:
	HeartbeatMonitor();
	virtual ~HeartbeatMonitor();

	virtual void    beat(int sequence_number, 
				const Ice::Current& /* current */);
	virtual void	interval(float intvl,
				const Ice::Current& /* current */);
	virtual void    stop(const Ice::Current& /* current */);
	int	multiplier() const { return _multiplier; }
	void	multiplier(int m);

public slots:
	void	timeout();
	void	stop_timer();
	void	start_timer(int milliseconds);
	
signals:
	void	update(QString);
	void	lost();
	void	reconnected();
	void	stop_timer_signal();
	void	start_timer_signal(int milliseconds);
};

} // namespace snowgui

#endif /* _HeartbeatMonitor_h */
