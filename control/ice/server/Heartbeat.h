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
void	callback_adapter<HeartbeatMonitorPrx>(HeartbeatMonitorPrx& p,
		const astro::callback::CallbackDataPtr d);

/**
 * \brief class to implement the heartbeat server
 *
 * This class has its own thread 
 */
class Heartbeat {
	int	_sequence_number;
	int	_interval;
	bool	_terminate;
	const static int	_default_interval;
	std::thread	_thread;
	std::condition_variable	_cond;
	std::mutex	_mutex;
	void	send();
public:
	Heartbeat(int interval = 5);
	virtual ~Heartbeat();
	void	run();
	int	interval() const { return _interval; }
	void	interval(int i);
	void	terminate(bool t);
	int	sequence_number() const { return _sequence_number; }
private:
	SnowCallback<HeartbeatMonitorPrx>	callbacks;
public:
	void	doregister(const Ice::Identity& heartbeatmonitor,
			const Ice::Current& current);
	void	unregister(const Ice::Identity& heartbeatmonitor,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _Heartbeat_h */
