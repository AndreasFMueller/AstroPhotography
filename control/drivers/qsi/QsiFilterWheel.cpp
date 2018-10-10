/*
 * QsiFilterWheel.cpp -- QSI filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiFilterWheel.h>
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

	// member initialization
	lastPosition = 0;
	lastState = unknown;
}

/**
 * \brief Destroy the filter wheel
 */
QsiFilterWheel::~QsiFilterWheel() {
}

/**
 * \brief Number of filters
 */
unsigned int	QsiFilterWheel::nFilters() {
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
		_camera.camera().get_Position(&position);
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
 * \brief Select a particular filter
 *
 * \param filterindex	index of the filter to select
 */
void	QsiFilterWheel::select(size_t filterindex) {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	if (filterindex >= nfilters) {
		throw std::invalid_argument("filter index too large");
	}
	short	position = filterindex;
	_camera.camera().put_Position(position);
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
 * \brief Get the state of th efilter wheel
 */
FilterWheel::State	QsiFilterWheel::getState() {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return lastState;
	}
	try {
		short	position = 0;
		_camera.camera().get_Position(&position);
		if (position < 0) {
			return lastState = FilterWheel::moving;
		}
		return lastState = FilterWheel::idle;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get current state: %s",
			x.what());
	}
	return lastState;
}

} // namespace qsi
} // namespace camera
} // namespace astro
