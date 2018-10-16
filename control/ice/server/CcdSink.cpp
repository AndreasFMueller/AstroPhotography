/*
 * CcdSink.cpp -- Implementation of the CcdSink class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <Ice/Connection.h>
#include <IceConversions.h>
#include <AstroUtils.h>

namespace snowstar {

/**
 * \brief constructor for the CcdSink
 *
 * The constructor creates the ImageSink proxy via which it will talk
 * to the client
 */
CcdSink::CcdSink(astro::camera::CcdPtr ccd, const Ice::Identity& identity,
		const Ice::Current& current) : _ccd(ccd) {
	std::string	is = identity.name + "@" + identity.category;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a CcdSink: %s", is.c_str());
	Ice::ObjectPrx	oneway = current.con->createProxy(identity)
					->ice_oneway();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "proxy created");
	sinkprx = ImageSinkPrx::uncheckedCast(oneway);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cast completed");
}

/**
 * \brief Image sink main method
 *
 * The operator() implementation absorbs ImageQueueEntries from the camera
 * converts them to ImageQueueEntries for the ICE protocol and sends them
 * to the client.
 */
void	CcdSink::operator()(const astro::camera::ImageQueueEntry& entry) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "operator()(ImageQueueEntry&) called");
	// don't do anything if we have no proxy (this should not happen,
	// we play safe here)
	if (sinkprx) {
		ImageQueueEntryPtr	e = convert(entry);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image: %s, size = %ld",
			entry.exposure.toString().c_str(),
			e->imagedata.size());
		try {
			sinkprx->image(*e);
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"cannot send image: %s %s",
				astro::demangle(typeid(x).name()).c_str(),
				x.what());
			sinkprx = NULL;
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ImageQueueEntry: sink stalled");
		_ccd->stopStream();
	}
}

/**
 * \brief stop operation
 *
 * The stop operation tells the client that no more images will be forthcoming
 * and that it can remove the adapter
 */
void	CcdSink::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop() called");
	if (sinkprx) {
		try {
			sinkprx->stop();
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stop: %s %s",
				astro::demangle(typeid(x).name()).c_str(),
				x.what());
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop: sink stalled");
		_ccd->stopStream();
	}
}

} // namespace snowstar
