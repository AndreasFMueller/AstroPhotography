/*
 * Heartbeat.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _Heartbeat_h
#define _Heartbeat_h

#include <types.h>
#include <CallbackHandler.h>
#include <thread>
#include <mutex>

namespace snowstar {

template<>
void	callback_adapter<HeartbeatMonitorPrx>(HeartbeatMonitorPrx p,
		const astro::callback::CallbackDataPtr d);

/**
 * \brief class to implement the heartbeat server
 *
 * This class has its own thread 
 */
class Heartbeat {
	int	_sequence_number;
	float	_interval;
	bool	_terminate;
	const static int	_default_interval;
	std::thread	_thread;
	std::condition_variable	_cond;
	std::mutex	_mutex;
	void	send();
	bool	_paused;
public:
	Heartbeat(float interval = 5);
	virtual ~Heartbeat();
	void	run();
	float	interval() const { return _interval; }
	void	send_interval();
	void	interval(float f);
	void	terminate(bool t);
	int	sequence_number() const { return _sequence_number; }
private:
	SnowCallback<HeartbeatMonitorPrx>	callbacks;
public:
	void	doregister(const Ice::Identity& heartbeatmonitor,
			const Ice::Current& current);
	void	unregister(const Ice::Identity& heartbeatmonitor,
			const Ice::Current& current);
	bool	paused() const { return _paused; }
	void	pause();
	void	resume();
};

} // namespace snowstar

#endif /* _Heartbeat_h */
