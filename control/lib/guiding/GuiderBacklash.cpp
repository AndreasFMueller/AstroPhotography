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
		if (!data) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no data");
			return data;
		}
		CallbackBacklashPoint	*p
			= dynamic_cast<CallbackBacklashPoint*>(&*data);
		if (NULL != p) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new point");
			_guider.GuiderBase::callback(p->data());
		}
		CallbackBacklashResult	*r
			= dynamic_cast<CallbackBacklashResult*>(&*data);
		if (NULL != r) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new results");
			_guider.GuiderBase::callback(r->data());
		}
		return data;
	}
};

/**
 * \brief start the backlash thread
 */
void	Guider::startBacklash(TrackerPtr tracker, double interval,
		backlash_t direction) {
	// check that we have everything
	if (!hasGuideport()) {
		throw std::runtime_error("no guide port");
	}
	_backlashdata.points.clear();
	_backlashdata.result.clear();
	_backlashdata.result.direction = direction;
        _state.startBacklash();
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

/**
 * \brief Stop the backlash process
 */
void	Guider::stopBacklash() {
	if (!_backlashthread) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no backlash thread");
		return;
	}
	if (_backlashthread->isrunning()) {
		_backlashthread->stop();
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "backlash thread not running");
	}
}

/**
 * \brief Tell the guider how many points to evaluate
 */
void	Guider::setLastPoints(int n) {
	_backlashwork->lastPoints(n);
}

} // namespace guiding
} // namespace astro
