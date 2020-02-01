/*
 * EventHandlerI.h -- event handler servant declaration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _EventHandlerI_h
#define _EventHandlerI_h

#include <types.h>
#include <AstroCallback.h>
#include <CallbackHandler.h>
#include <StatisticsI.h>

namespace snowstar {

template<>
void    callback_adapter<EventMonitorPrx>(EventMonitorPrx& p,
                const astro::callback::CallbackDataPtr d);

class EventHandlerI : virtual public EventHandler, public StatisticsI {
	astro::callback::CallbackPtr	_callback;
public:
	EventHandlerI();
	virtual ~EventHandlerI();
	virtual Event	eventId(int id, const Ice::Current& current);
	virtual eventlist	eventsBetween(double fromago, double toago,
					const Ice::Current& current);
	virtual eventlist	eventsCondition(const std::string& condition,
					const Ice::Current& current);
	virtual void	registerMonitor(const Ice::Identity& eventmonitor,
				const Ice::Current& current);
	virtual void	unregisterMonitor(const Ice::Identity& eventmonitor,
				const Ice::Current& current);
	void	update(astro::callback::CallbackDataPtr data);
	SnowCallback<EventMonitorPrx>     eventcallbacks;
};

} // namespace snowstar

#endif /* _EventHandlerI_h */
