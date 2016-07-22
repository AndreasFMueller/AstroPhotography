/*
 * CcdSink.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <Ice/Connection.h>
#include <IceConversions.h>

namespace snowstar {

CcdSink::CcdSink(const Ice::Identity& identity, const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a CcdSink");
	Ice::ObjectPrx	oneway = current.con->createProxy(identity)
					->ice_oneway();
	sinkprx = ImageSinkPrx::uncheckedCast(oneway);
}

void	CcdSink::operator()(const astro::camera::ImageQueueEntry& entry) {
	ImageQueueEntryPtr	e = convert(entry);
	sinkprx->image(*e);
}

void	CcdSink::stop() {
	sinkprx->stop();
}

} // namespace snowstar
