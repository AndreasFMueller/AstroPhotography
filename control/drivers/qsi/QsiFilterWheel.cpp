/*
 * QsiFilterWheel.cpp -- QSI filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiFilterWheel.h>

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
}

QsiFilterWheel::~QsiFilterWheel() {
}

unsigned int	QsiFilterWheel::nFilters() {
	return nfilters;
}

unsigned int	QsiFilterWheel::currentPosition() {
	short	position = 0;
	_camera.camera().get_Position(&position);
	return position;
}

void	QsiFilterWheel::select(size_t filterindex) {
	if (filterindex >= nfilters) {
		throw std::invalid_argument("filter index too large");
	}
	short	position = filterindex;
	_camera.camera().put_Position(position);
}

std::string	QsiFilterWheel::filterName(size_t filterindex) {
	if (filterindex >= nfilters) {
		throw std::invalid_argument("filter index too large");
	}
	std::string	*names = new std::string[nfilters];
	_camera.camera().get_Names(names);
	std::string	filtername(names[filterindex]);
	delete[] names;
	return filtername;
}

} // namespace qsi
} // namespace camera
} // namespace astro
