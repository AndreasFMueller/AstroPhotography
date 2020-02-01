/*
 * GatewayI.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _GatewayI_h
#define _GatewayI_h

#include <types.h>
#include <AstroCallback.h>
#include <CallbackHandler.h>
#include <StatisticsI.h>

namespace snowstar {

template<>
void	callback_adapter<StatusUpdateMonitorPrx>(StatusUpdateMonitorPrx& p,
		const astro::callback::CallbackDataPtr d);

class GatewayI : virtual public Gateway, public StatisticsI {
public:
	GatewayI();
	virtual ~GatewayI();
	virtual void	send(const StatusUpdate& update,
				const Ice::Current& current);
	virtual void    registerMonitor(
				const Ice::Identity& statusupdatemonitor,
				const Ice::Current& current);
	virtual void    unregisterMonitor(
				const Ice::Identity& statusupdatemonitor,
				const Ice::Current& current);
	void	update(const StatusUpdate& statusupdatedata);
	SnowCallback<StatusUpdateMonitorPrx>	statusupdatecallbacks;
};

} // namespace snowstar

#endif /* _GatewayI_h */
