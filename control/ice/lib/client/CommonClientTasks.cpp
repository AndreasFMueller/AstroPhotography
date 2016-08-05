/*
 * CommonClientTask.cpp -- implementation of common client task
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommonClientTasks.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <math.h>
#include <IceConversions.h>
#include <IceUtil/UUID.h>

namespace snowstar {

//////////////////////////////////////////////////////////////////////
// CcdTask
//////////////////////////////////////////////////////////////////////
CcdTask::CcdTask(CcdPrx& ccd) : _ccd(ccd) {
}

void	CcdTask::frame(const astro::image::ImageRectangle& frame) {
	astro::camera::CcdInfo	ccdinfo = convert(_ccd->getInfo());
	if ((frame.size().width() == 0) || (frame.size().height() == 0)) {
		_exposure.frame(ccdinfo.getFrame());
	} else {
		_exposure.frame(ccdinfo.clipRectangle(frame));
	}
}

void	CcdTask::frame(const std::string& framespec) {
	if (framespec.size() > 0) {
		frame(astro::image::ImageRectangle(framespec));
	}
}

void	CcdTask::binning(const astro::image::Binning& binning) {
	_exposure.mode(binning);
}

void	CcdTask::binning(const std::string& binning) {
	if (binning.size() > 0) {
		_exposure.mode(astro::image::Binning(binning));
	}
}

void	CcdTask::exposuretime(double exposuretime) {
	_exposure.exposuretime(exposuretime);
}

void	CcdTask::purpose(const astro::camera::Exposure::purpose_t purpose) {
	_exposure.purpose(purpose);
	_exposure.shutter((purpose == astro::camera::Exposure::dark)
				? astro::camera::Shutter::CLOSED
				: astro::camera::Shutter::OPEN);
}

void	CcdTask::purpose(const std::string& purposename) {
	purpose(astro::camera::Exposure::string2purpose(purposename));
}

void	CcdTask::shutter(astro::camera::Shutter::state shutter) {
	_exposure.shutter(shutter);
}

Exposure	CcdTask::exposure() const {
	return convert(_exposure);
}

void	CcdTask::start() {
	_ccd->startExposure(exposure());
}

/**
 * \brief Wait until ccd becomes available for expsure
 */
void	CcdTask::available(int timeout) {
	time_t	end;
	time(&end);
	end += timeout;
	if (snowstar::EXPOSING == _ccd->exposureStatus()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "need to cancel an exposure");
                _ccd->cancelExposure();
		time_t	now;
		do {
                        usleep(100000);
			time(&now);
		} while ((snowstar::IDLE != _ccd->exposureStatus())
			&& (now < end));
		if ((snowstar::IDLE != _ccd->exposureStatus()) 
			&& (snowstar::EXPOSED != _ccd->exposureStatus())) {
			throw std::runtime_error("cancel did not work");
		}
        }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd now available");
}

/**
 * \brief Wait for the exposure to complete
 */
void	CcdTask::wait(int timeout) {
	useconds_t	t = 1000000 * _exposure.exposuretime();
	usleep(t);
	time_t	end;
	time(&end);
	end += timeout;
	time_t	now;
	do {
		usleep(100000);
		time(&now);
	} while ((_ccd->exposureStatus() == EXPOSING) && (now < end));
	if (_ccd->exposureStatus() != EXPOSED) {
		throw std::runtime_error("exposure did not complete");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for camera complete");
}

//////////////////////////////////////////////////////////////////////
// CoolerTask implementation
//////////////////////////////////////////////////////////////////////
void	CoolerTask::setup(double temperature) {
	// initialize member variables
	_stop_on_exit = false;
	we_turned_cooler_on = false;
	_absolute = 273.15 + temperature;

	// check whether there is something to do
	if (!_cooler) {
		return;
	}

	// make sure we have a temperature at all
	if (!(temperature == temperature)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no temperature set, leave cooler alone");
		return;
	}

	// ensure we have a good temperature
	if (_absolute < 0) {
		std::string	msg = astro::stringprintf("bad absolute "
			"temperature %.2fK", _absolute);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// set the temperature
	_cooler->setTemperature(_absolute);

	// start the cooler and remember we did so to later turn it off again
	if (!_cooler->isOn()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler on");
		_cooler->setOn(true);
		we_turned_cooler_on = true;
	}
}

/**
 * \brief start the cooler
 */
CoolerTask::CoolerTask(CoolerPrx cooler, double temperature)
	: _cooler(cooler) {
	setup(temperature);
}

CoolerTask::CoolerTask(RemoteInstrument& ri, double temperature) {
	if (ri.has(InstrumentCooler)) {
		_cooler = ri.cooler();
	}
	setup(temperature);
}

/**
 * \brief Wait for the temperature to be reached
 */
void	CoolerTask::wait(int timeout) {
	if (!_cooler) {
		return;
	}
	if (!_cooler->isOn()) {
		std::string	msg = astro::stringprintf("cooler %s not on, "
			"cannot wait for it", _cooler->getName().c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		return;
	}
	double	delta;
	time_t	end;
	time(&end);
	end += timeout;
	time_t	now;
	do {
		sleep(1);
		double	actual = _cooler->getActualTemperature();
		delta = fabs(_absolute - actual);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"set: %.1f, actual: %1.f, delta: %.1f",
			_absolute, actual, delta);
		time(&now);
	} while ((delta > 1) && (now < end));
	if (delta > 1) {
		throw std::runtime_error("failed to reach temperature");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "temperature reached");
}

/**
 * \brief Turn of the cooler
 */
CoolerTask::~CoolerTask() {
	if (!_cooler) {
		return;
	}
	if (!(_absolute == _absolute)) {
		return;
	}
	if (!we_turned_cooler_on) {
		return;
	}
	if (_stop_on_exit) {
		stop();
	}
}

void	CoolerTask::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler off");
	_cooler->setOn(false);
}

//////////////////////////////////////////////////////////////////////
// FocuserTask implementation
//////////////////////////////////////////////////////////////////////
void	FocuserTask::setup() {
	we_started_focuser = false;

	// if there is no focuser, no need to worry
	if (!_focuser) {
		return;
	}

	// ensure that the position is a valid one for the focuser
	int	min = _focuser->min();
	int	max = _focuser->max();
	if ((_position < min) || (_position > max)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "position %d not "
			"between %d and %d", _position, min, max);
		return;
	}

	// set the focuser to the position
	_focuser->set(_position);
	we_started_focuser = true;
}

FocuserTask::FocuserTask(FocuserPrx focuser, int position)
	: _focuser(focuser), _position(position) {
}

FocuserTask::FocuserTask(RemoteInstrument& ri, int position)
	: _position(position) {
	if (ri.has(InstrumentFocuser)) {
		_focuser = ri.focuser();
	}
	setup();
}

void	FocuserTask::wait(int timeout) {
	if (!_focuser) {
		return;
	}
	// if we did not start the focuser, then it is already at the right
	// position, no need to wait
	if (!we_started_focuser) {
		return;
	}
	// wait for the position to be reached
	time_t	end;
	time(&end);
	end += timeout;
	time_t	now;
	int	delta = 0;
	do {
		usleep(100000);
		int	current = _focuser->current();
		delta = _position - current;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "current = %d, target = %d, "
			"delta = %d", current, _position, delta);
		time(&now);
	} while ((delta != 0) && (now < end));
	if (delta != 0) {
		throw std::runtime_error("could not reach focuser position");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus position reached");
}

//////////////////////////////////////////////////////////////////////
// FilterwheelTask implementation
//////////////////////////////////////////////////////////////////////
void	FilterwheelTask::setup() {
	we_started_filterwheel = false;
	// if there is no filterwheel, we should return immediately
	if (!_filterwheel) {
		return;
	}

	// find out whether filter wheel position can be set
	if (_filtername.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no filter name, returning");
		return;
	}

	// set filter wheel position
	_filterwheel->selectName(_filtername);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set filter name to %s",
		_filtername.c_str());
	we_started_filterwheel = true;
}

FilterwheelTask::FilterwheelTask(FilterWheelPrx filterwheel,
	const std::string& filtername)
	: _filterwheel(filterwheel), _filtername(filtername) {
	setup();
}

FilterwheelTask::FilterwheelTask(RemoteInstrument& ri,
	const std::string& filtername) : _filtername(filtername) {
	if (ri.has(InstrumentFilterWheel)) {
		_filterwheel = ri.filterwheel();
	}
	setup();
}

void	FilterwheelTask::wait(int timeout) {
	// if there is no filterwheel, we can return immediately
	if (!_filterwheel) {
		return;
	}

	// in case we started the filterwheel, we should at least wait
	// for one second to make sure the filter wheel has changed state
	if (we_started_filterwheel) {
		sleep(1);
	}
	we_started_filterwheel = false;

	// wait for filter wheel to stabilize (i.e. get back to IDLE)
	time_t	end;
	time(&end);
	end += timeout;
	time_t	now;
	do {
		usleep(100000);
		time(&now);
	} while ((_filterwheel->getState() != FwIDLE) && (now < end));
	if (_filterwheel->getState() != FwIDLE) {
		throw std::runtime_error("filterwheel did not stabilize");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel is idle again");
}

//////////////////////////////////////////////////////////////////////
// Client Callback adapter
//////////////////////////////////////////////////////////////////////
CallbackAdapter::CallbackAdapter(Ice::CommunicatorPtr communicator) {
	_adapter = communicator->createObjectAdapter("");
	_adapter->activate();
}

void	CallbackAdapter::connect(IceProxy::Ice::Object& proxy) {
	proxy.ice_getConnection()->setAdapter(_adapter);
}

Ice::Identity	CallbackAdapter::add(Ice::ObjectPtr callback) {
	Ice::Identity	ident;
	ident.name = IceUtil::generateUUID();
	ident.category = "";
	_adapter->add(callback, ident);
	return ident;
}

} // namespace snowstar
