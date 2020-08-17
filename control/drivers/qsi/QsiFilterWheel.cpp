/*
 * QsiFilterWheel.cpp -- QSI filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiFilterWheel.h>
#include <QsiUtils.h>
#include <AstroExceptions.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <sstream>

namespace astro {
namespace camera {
namespace qsi {

/**
 * \brief Construct the QsiFilterWheel
 *
 * \param camera	camera containing the filter wheel.
 */
QsiFilterWheel::QsiFilterWheel(QsiCamera& camera)
	: FilterWheel(DeviceName(camera.name(), DeviceName::Filterwheel,
		"filterwheel")),
	  _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construction of QSI filterwheel");
	// lock the device
	std::lock_guard<std::recursive_mutex>	lock(_camera.mutex);

	// initialize the thread variables used for filter movement
	_thread = NULL;
	_movement_done = true;

	// get the number of filters
	int	filtercount = 0;
	_camera.camera().get_FilterCount(filtercount);
	nfilters = filtercount;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel has %d filters", nfilters);

	// retrieve filter names
	std::string	*names = new std::string[nfilters];
	_camera.camera().get_Names(names);
	for (unsigned int index = 0; index < nfilters; index++) {
		filternames.push_back(names[index]);
	}
	delete[] names;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d filter names",
		filternames.size());

	// report the filters we have found
	std::ostringstream	out;
	std::vector<std::string>::const_iterator	i;
	for (auto i = filternames.begin(); i != filternames.end(); i++) {
		out << " " << *i;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filters: %s", out.str().c_str());

	// state member initialization
	lastPosition = 0;
	lastState = unknown;
}

/**
 * \brief Destroy the filter wheel
 */
QsiFilterWheel::~QsiFilterWheel() {
	// make sure the thread has terminated and is properly destroyed
	std::lock_guard<std::recursive_mutex>	lock(_camera.mutex);
	if (_thread) {
		_thread->join();
		delete _thread;
	}
}

/**
 * \brief Number of filters
 */
unsigned int	QsiFilterWheel::nFilters0() {
	return nfilters;
}

/**
 * \brief find the current position
 */
unsigned int	QsiFilterWheel::currentPosition() {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return lastPosition;
	}
	try {
		short	position = 0;
		START_STOPWATCH;
		_camera.camera().get_Position(&position);
		END_STOPWATCH("get_Position");
		if (position < 0) {
			throw astro::camera::BadState("filter wheel moving");
		}
		lastPosition = position;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get the position: %s",
			x.what());
	}
	return lastPosition;
}

/**
 * \brief Move the filterwheel to a new position
 *
 * \param newposition	position we have to move to
 */
void	QsiFilterWheel::move(size_t newposition) {
	callback(FilterWheel::moving);
	std::lock_guard<std::recursive_mutex>	lock(_camera.mutex);
	short	position = newposition;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "put position %d", position);
	START_STOPWATCH;
	_camera.camera().put_Position(position);
	END_STOPWATCH("put_Position()");
	position = 0;
	_camera.camera().get_Position(&position);
	if ((size_t)position != newposition) {
		debug(LOG_ERR, DEBUG_LOG, 0, "wrong position");
		lastState = FilterWheel::unknown;
	}
	_movement_done = true;
	lastState = FilterWheel::idle;
	callback(position);
	callback(FilterWheel::idle);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "put position return");
}

/**
 * \brief Main method for the thread
 *
 * \param filterwheel 	FilterWheel object on which this thread operates
 * \param newposition	the filterwheel position to move to
 */
void	moveposition(QsiFilterWheel *filterwheel, size_t newposition) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch filterwheel thread");
	filterwheel->move(newposition);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel thread completes");
}

/**
 * \brief Select a particular filter
 *
 * The put_Position method blocks until the filterwheel settles.
 * To prevent clients from stalling, we therefore launch a separate
 * thread which in turn calls the put_Position method. Until the thread
 * completes, the state FilterWheel::moving is returned.
 *
 * \param filterindex	index of the filter to select
 */
void	QsiFilterWheel::select(size_t filterindex) {
	// consistency checking
	if (filterindex >= nfilters) {
		throw std::invalid_argument("filter index too large");
	}

	// lock the filterwheel
	std::lock_guard<std::recursive_mutex>	lock(_camera.mutex);

	// find the filterwheel state, if it is moving, we cannot
	// initiate another move, i.e. we want to throw a bad state
	// exception
	switch (getState()) {
	case FilterWheel::idle:
		break;
	case FilterWheel::moving:
		throw BadState("filterwheel already moving");
	case FilterWheel::unknown:
		throw BadState("filterwheel in unknown state");
	}
	// if we get here, then the filterwheel is idle.

	// start moving
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"start a thread to move the filter wheel");
	lastState = FilterWheel::moving;
	_movement_done = false;
	_thread = new std::thread(moveposition, this, filterindex);
}

/**
 * \brief Select a filter by name
 *
 * \param filtername	name of the filter
 */
void	QsiFilterWheel::select(const std::string& filtername) {
	for (unsigned int index = 0; index < nfilters; index++) {
		if (filternames[index] == filtername) {
			select(index);
			return;
		}
	}
	try {
		select(std::stoi(filtername));
	} catch (...) {
	}
	std::string	msg = stringprintf("filter '%s' not found",
		filtername.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get the filter name for a position
 *
 * \param index of filter position
 */
std::string	QsiFilterWheel::filterName(size_t filterindex) {
	if (filterindex >= nfilters) {
		throw std::invalid_argument("filter index too large");
	}
	return filternames[filterindex];
}

/**
 * \brief Get the state of the filter wheel
 */
FilterWheel::State	QsiFilterWheel::getState() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "getState()");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return lastState;
	}

	// first cleanup a thread that has already completed
	if ((NULL != _thread) && (_movement_done)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cleanup filterwheel thread");
		_thread->join();
		delete _thread;
		_thread = NULL;
	}

	try {
		// check the position
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "check position");
		short	position = 0;
		START_STOPWATCH;
		_camera.camera().get_Position(&position);
		END_STOPWATCH("get_Position()");
		if (position < 0) {
			return lastState = FilterWheel::moving;
		}
		return lastState = FilterWheel::idle;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get current state: %s",
			x.what());
		lastState = FilterWheel::unknown;
	}
	return lastState;
}

} // namespace qsi
} // namespace camera
} // namespace astro
