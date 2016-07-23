/*
 * CcdSink.cpp -- Implementation of the CcdSink class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <Ice/Connection.h>
#include <IceConversions.h>

namespace snowstar {

/**
 * \brief constructor for the CcdSink
 *
 * The constructor creates the ImageSink proxy via which it will talk
 * to the client
 */
CcdSink::CcdSink(const Ice::Identity& identity, const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a CcdSink");
	Ice::ObjectPrx	oneway = current.con->createProxy(identity)
					->ice_oneway();
	sinkprx = ImageSinkPrx::uncheckedCast(oneway);
}

/**
 * \brief Image sink main method
 *
 * The operator() implementation absorbs ImageQueueEntries from the camera
 * converts them to ImageQueueEntries for the ICE protocol and sends them
 * to the client.
 */
void	CcdSink::operator()(const astro::camera::ImageQueueEntry& entry) {
	// don't do anything if we have no proxy (this should not happen,
	// we play safe here)
	if (sinkprx) {
		ImageQueueEntryPtr	e = convert(entry);
		sinkprx->image(*e);
	}
}

/**
 * \brief stop operation
 *
 * The stop operation tells the client that no more images will be forthcoming
 * and that it can remove the adapter
 */
void	CcdSink::stop() {
	if (sinkprx) {
		sinkprx->stop();
	}
}

} // namespace snowstar
