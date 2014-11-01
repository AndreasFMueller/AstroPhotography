/*
 * QsiFilterWheel.cpp -- QSI filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiFilterWheel.h>
#include <AstroExceptions.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace qsi {

QsiFilterWheel::QsiFilterWheel(QsiCamera& camera)
	: FilterWheel(DeviceName(camera.name(), DeviceName::Filterwheel,
		"filterwheel")),
	  _camera(camera) {
	int	filtercount = 0;
	_camera.camera().get_FilterCount(filtercount);
	nfilters = filtercount;
	// retrieve filter names
	std::string	*names = new std::string[nfilters];
	_camera.camera().get_Names(names);
	for (unsigned int index = 0; index < nfilters; index++) {
		filternames.push_back(names[index]);
	}
	delete[] names;
}

QsiFilterWheel::~QsiFilterWheel() {
}

unsigned int	QsiFilterWheel::nFilters() {
	return nfilters;
}

unsigned int	QsiFilterWheel::currentPosition() {
	short	position = 0;
	_camera.camera().get_Position(&position);
	if (position < 0) {
		throw astro::camera::BadState("filter wheel moving");
	}
	return position;
}

void	QsiFilterWheel::select(size_t filterindex) {
	if (filterindex >= nfilters) {
		throw std::invalid_argument("filter index too large");
	}
	short	position = filterindex;
	_camera.camera().put_Position(position);
}

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

std::string	QsiFilterWheel::filterName(size_t filterindex) {
	if (filterindex >= nfilters) {
		throw std::invalid_argument("filter index too large");
	}
	return filternames[filterindex];
}

FilterWheel::State	QsiFilterWheel::getState() {
	short	position = 0;
	_camera.camera().get_Position(&position);
	if (position < 0) {
		return FilterWheel::moving;
	}
	return FilterWheel::idle;
}

} // namespace qsi
} // namespace camera
} // namespace astro
