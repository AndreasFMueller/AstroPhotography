/*
 * GuiderBacklash.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <Backlash.h>

namespace astro {
namespace guiding {

/**
 * \brief BacklashPoint Callback to install in the guider class
 */
class BacklashCallback : public callback::Callback {
	Guider&	_guider;
public:
	BacklashCallback(Guider& guider) : _guider(guider) { }
	CallbackDataPtr	operator()(CallbackDataPtr data) {
		CallbackBacklashPoint	*p
			= dynamic_cast<CallbackBacklashPoint*>(&*data);
		if (!p) {
			_guider.GuiderBase::callback(p->data());
		}
		CallbackBacklashResult	*r
			= dynamic_cast<CallbackBacklashResult*>(&*data);
		if (!r) {
			_guider.GuiderBase::callback(r->data());
		}
		return data;
	}
};

/**
 * \brief start the backlash thread
 */
void	Guider::startBacklash(TrackerPtr tracker, double interval) {
	// check that we have everything
	if (!hasGuideport()) {
		throw std::runtime_error("no guide port");
	}
	try {
		// create the work object
		BacklashWork	*backlashwork = new BacklashWork(imager(),
			tracker, guideport());
		_backlashwork	= BacklashWorkPtr(backlashwork);

		// configure the intervaland the exposure
		_backlashwork->interval(interval);
		_backlashwork->exposure(exposure());

		// create the callbacks and install them
		CallbackPtr _callback(new BacklashCallback(*this));
		_backlashwork->callback(_callback);

		// create the backlash thread
		BacklashThread	*backlashthread
			= new BacklashThread(backlashwork);
		_backlashthread = BacklashThreadPtr(backlashthread);

		// start the thread
		_backlashthread->start();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "exception in backlash start: %s",
			x.what());
		throw;
	}
}

BacklashDataPtr	Guider::backlashData() {
	// XXX needs implementation
	return BacklashDataPtr(NULL);
}

/**
 * \brief Stop the backlash process
 */
void	Guider::stopBacklash() {
	if (!_backlashthread) {
		return;
	}
	if (!_backlashthread->isrunning()) {
		_backlashthread->stop();
	}
}

} // namespace guiding
} // namespace astro
