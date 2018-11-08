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
	StatusUpdate	*sap = dynamic_cast<StatusUpdate*>(&*data);
	if (NULL == sap) {
		return;
	}
	p->update(*sap);
}

GatewayI::GatewayI() {
}

GatewayI::~GatewayI() {
}

void	GatewayI::send(const StatusUpdate& statusupdate,
			const Ice::Current& /* current */) {
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
	astro::task::TaskUpdateCallbackData	*cbd
		= new astro::task::TaskUpdateCallbackData(convert(su));
	astro::callback::CallbackDataPtr	data(cbd);
	statusupdatecallbacks(data);
}

} // namespace snowstar
