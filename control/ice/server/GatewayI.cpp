/*
 * GatewayI.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <GatewayI.h>
#include <AstroTask.h>
#include <IceConversions.h>

namespace snowstar {

template<>
void	callback_adapter<StatusUpdateMonitorPrx>(StatusUpdateMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "message type: %s",
		astro::demangle(typeid(*data).name()).c_str());
	astro::task::TaskUpdateCallbackData	*sap
		= dynamic_cast<astro::task::TaskUpdateCallbackData*>(&*data);
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
			astro::demangle(typeid(*data).name()).c_str());
		astro::task::TaskUpdateCallbackData	*tucd
			= dynamic_cast<astro::task::TaskUpdateCallbackData*>(&*data);
		if (NULL != tucd) {
			_gateway->update(convert(tucd->data()));
		}
		return data;
	}
	void	stop() const { }
};

GatewayI::GatewayI() {
	astro::callback::CallbackPtr	callback(new TaskUpdateForwarder(this));
	astro::task::Gateway::setCallback(callback);
}

GatewayI::~GatewayI() {
	astro::callback::CallbackPtr	callback;
	astro::task::Gateway::setCallback(callback);
}

void	GatewayI::send(const StatusUpdate& statusupdate,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a status udpate");
	update(statusupdate);
}

void    GatewayI::registerMonitor(const Ice::Identity& statusupdatemonitor,
			const Ice::Current& current) {
	statusupdatecallbacks.registerCallback(statusupdatemonitor, current);
}

void    GatewayI::unregisterMonitor(const Ice::Identity& statusupdatemonitor,
			const Ice::Current& current) {
	statusupdatecallbacks.unregisterCallback(statusupdatemonitor, current);
}

void	GatewayI::update(const StatusUpdate& su) {
	astro::task::TaskUpdate	tu = convert(su);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending status update %s",
		tu.toString(std::string("\n")).c_str());
	astro::task::TaskUpdateCallbackData	*cbd
		= new astro::task::TaskUpdateCallbackData(convert(su));
	astro::callback::CallbackDataPtr	data(cbd);
	statusupdatecallbacks(data);
}

} // namespace snowstar
