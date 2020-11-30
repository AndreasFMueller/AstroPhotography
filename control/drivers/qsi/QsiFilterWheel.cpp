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
 * During the construction of the filterwheel object, the names of the
 * filters are retrieved through the API. But the names are not stored
 * in the camera, but instead in ~/.QSIConfig configuration file in the
 * users home directory. This also means that the filter names need to
 * be set on each system on which the camera is used.
 *
 * \param camera	camera containing the filter wheel.
 */
QsiFilterWheel::QsiFilterWheel(QsiCamera& camera)
	: FilterWheel(DeviceName(camera.name(), DeviceName::Filterwheel)),
	  _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construction of QSI filterwheel");
	// lock the device
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

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
	threadwait();
}

/**
 * \brief Wait for the move thread to complete
 *
 * The movement of the filterwheel cannot be cancelled, so we have to wait
 * for completion. But since the movement usually last less than a second,
 * isn't really much of an issue.
 */
void	QsiFilterWheel::threadwait() {
	if (_thread.joinable()) {
		_thread.join();
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
 *
 * This method usually returns the cache filter position, the exception
 * being if the camera is in an unknown state, which it is after startup.
 * In that case the camera is queried and the filter wheel position is
 * retrieved from the camera.
 */
unsigned int	QsiFilterWheel::currentPosition() {
	FilterWheel::State	state = lastState;
	switch (state) {
	case idle:
		return lastPosition;
	case moving:
		throw BadState("filter wheel moving");
	case unknown:
		// fall through to query state
		break;
	}

	// at this point we need to get the state, but this ist most easily
	// done by querying the state. If the state is idle after we call
	// getState(), we also have a position
	switch (getState()) {
	case idle:
		return lastPosition;
	case moving:
	case unknown:
		throw BadState("filter wheel moving");
	}
	throw std::logic_error("should not get to this point in "
		"QsiFilterWheel::currentPosition()");
}

/**
 * \brief Move the filterwheel to a new position
 *
 * \param newposition	position we have to move to
 */
void	QsiFilterWheel::move(size_t newposition) {
	// lock the camera
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

	// change the state to moving
	callback(FilterWheel::moving);

	try {
		// send the new position to the camera
		short	position = newposition;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "put position %d", position);
		START_STOPWATCH;
		_camera.camera().put_Position(position);
		END_STOPWATCH("put_Position()");

		// read back the position to ensure that it has worked
		position = 0;
		_camera.camera().get_Position(&position);
		if ((size_t)position != newposition) {
			debug(LOG_ERR, DEBUG_LOG, 0, "wrong position: "
				"%hd != %u", position, newposition);
			callback(lastState = FilterWheel::unknown);
			return;
		}

		// everything is OK
		callback(lastState = FilterWheel::idle);
		lastPosition = position;
		callback(position);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("movement failed: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
}

/**
 * \brief Main method for the thread
 *
 * \param filterwheel 	FilterWheel object on which this thread operates
 * \param newposition	the filterwheel position to move to
 */
void	QsiFilterWheel::moveposition(QsiFilterWheel *filterwheel,
		size_t newposition) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch filterwheel thread");
	try {
		filterwheel->move(newposition);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread crashed by %s: %s",
			demangle_string(x).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread crashed");
	}
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

	// lock the camera, this i necessary to ensure that
	// no other thread starts moving the filterwheel while we are
	// interpreting the state
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

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

	// if we get here, then the filterwheel is idle. However, there
	// still could be a thread maybe in terminated state, so we do
	// the cleanup just for good measure
	threadwait();

	// start moving by starting the thread that does the moving
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"start a thread to move the filter wheel");
	lastState = FilterWheel::moving;
	_thread = std::thread(moveposition, this, filterindex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select method complete");
}

/**
 * \brief Select a filter by name
 *
 * \param filtername	name of the filter
 */
void	QsiFilterWheel::select(const std::string& filtername) {
	// try to find the filter name in the list of valid filter names
	for (unsigned int index = 0; index < nfilters; index++) {
		if (filternames[index] == filtername) {
			select(index);
			return;
		}
	}

	// interpret the filter name as a number
	int	index;
	try {
		index = std::stoi(filtername);
	} catch (...) {
		std::string	msg = stringprintf("filter '%s' not found",
			filtername.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// if the filtername was a number, try to select it
	select(index);
	return;
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
	// we only need to query the state if the last state is 'unknown'
	if (lastState != unknown) {
		return lastState;
	}

	//debug(LOG_DEBUG, DEBUG_LOG, 0, "getState()");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return lastState;
	}


	// if we get to this point the last known state was 'unknown', and
	// we are recovering from some accident that may have happend in 
	// the filterwheel thread. Normally, the filterwheel thread would
	// set the position and the state.

	// query the position
	try {
		// check the position
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "check position");
		short	position = 0;
		START_STOPWATCH;
		_camera.camera().get_Position(&position);
		END_STOPWATCH("get_Position()");
		if (position < 0) {
			lastState = FilterWheel::moving;
			callback(lastState);
			return lastState;
		}
		// if we get a position, then we remember this as the last
		// position and we inform all the callbacks
		lastPosition = position;
		callback(lastState = FilterWheel::idle);
		callback(lastPosition);
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
