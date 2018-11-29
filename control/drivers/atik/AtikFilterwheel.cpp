/*
 * AtikFilterwheel.cpp -- implementation of the ATIK filterwheel
 *
 * (c) 2106 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikFilterwheel.h>
#include <AtikUtils.h>

namespace astro {
namespace camera {
namespace atik {

/**
 * \brief Create a filter wheel
 *
 * \param camera	camera to which this filterwheel is attached
 */
AtikFilterwheel::AtikFilterwheel(::AtikCamera *camera)
	: FilterWheel(filterwheelname(camera)), _camera(camera) {
}

/**
 * \brief Find the number of filters
 *
 * \return the number filters this camera has
 */
unsigned int	AtikFilterwheel::nFilters0() {
	unsigned int	filtercount;
	bool	moving;
	unsigned int	current;
	unsigned int	target;
	_camera->getFilterWheelStatus(&filtercount, &moving, &current, &target);
	return filtercount;
}

/**
 * \brief Get the current position
 *
 * \return filter position (integer between 0 - (nFilters()-1) )
 */
unsigned int	AtikFilterwheel::currentPosition() {
	unsigned int	filtercount;
	bool	moving;
	unsigned int	current;
	unsigned int	target;
	_camera->getFilterWheelStatus(&filtercount, &moving, &current, &target);
	return current;
}

/**
 * \brief Get the current state of the filterwheel
 *
 * \return current filter wheel state
 */
FilterWheel::State	AtikFilterwheel::getState() {
	unsigned int	filtercount;
	bool	moving;
	unsigned int	current;
	unsigned int	target;
	_camera->getFilterWheelStatus(&filtercount, &moving, &current, &target);
	if (moving) {
		return FilterWheel::moving;
	}
	return FilterWheel::idle;
}

/**
 * \brief Select a new filter
 *
 * \param filterindex	filter to select (0 - (nFilters()-1) )
 */
void	AtikFilterwheel::select(size_t filterindex) {
	_camera->setFilter(filterindex);
}

/**
 * \brief Get the user friendly name of the filter wheel
 */
std::string	AtikFilterwheel::userFriendlyName() const {
	if (_camera) {
		return std::string(_camera->getName());
	}
	return Device::userFriendlyName();
}

} // namespace atik
} // namespace camera
} // namespace stro
