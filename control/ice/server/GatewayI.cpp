/*
 * GatewayI.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <GatewayI.h>
#include <AstroGateway.h>
#include <IceConversions.h>

namespace snowstar {

template<>
void	callback_adapter<StatusUpdateMonitorPrx>(StatusUpdateMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "message type: %s",
		astro::demangle_cstr(*data));
	astro::gateway::TaskUpdateCallbackData	*sap
		= dynamic_cast<astro::gateway::TaskUpdateCallbackData*>(&*data);
	if (NULL == sap) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this is not a status update");
		return;
	}
	p->update(convert(sap->data()));
}

class TaskUpdateForwarder : public astro::callback::Callback {
	GatewayI	*_gateway;
public:
	TaskUpdateForwarder(GatewayI *gateway) : _gateway(gateway) { }
	CallbackDataPtr	operator()(CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "forwarding %s",
			astro::demangle_cstr(*data));
		astro::gateway::TaskUpdateCallbackData	*tucd
			= dynamic_cast<astro::gateway::TaskUpdateCallbackData*>(&*data);
		if (NULL != tucd) {
			_gateway->update(convert(tucd->data()));
		}
		return data;
	}
	void	stop() const { }
};

GatewayI::GatewayI() {
	astro::callback::CallbackPtr	callback(new TaskUpdateForwarder(this));
	astro::gateway::Gateway::setCallback(callback);
}

GatewayI::~GatewayI() {
	astro::callback::CallbackPtr	callback;
	astro::gateway::Gateway::setCallback(callback);
}

void	GatewayI::send(const StatusUpdate& statusupdate,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a status udpate");
	update(statusupdate);
}

void    GatewayI::registerMonitor(const Ice::Identity& statusupdatemonitor,
			const Ice::Current& current) {
	CallStatistics::count(current);
	statusupdatecallbacks.registerCallback(statusupdatemonitor, current);
}

void    GatewayI::unregisterMonitor(const Ice::Identity& statusupdatemonitor,
			const Ice::Current& current) {
	CallStatistics::count(current);
	statusupdatecallbacks.unregisterCallback(statusupdatemonitor, current);
}

void	GatewayI::update(const StatusUpdate& su) {
	astro::gateway::TaskUpdate	tu = convert(su);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending status update %s",
		tu.toString(std::string("\n")).c_str());
	astro::gateway::TaskUpdateCallbackData	*cbd
		= new astro::gateway::TaskUpdateCallbackData(convert(su));
	astro::callback::CallbackDataPtr	data(cbd);
	statusupdatecallbacks(data);
}

} // namespace snowstar
