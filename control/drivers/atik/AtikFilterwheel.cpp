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

AtikFilterwheel::AtikFilterwheel(::AtikCamera *camera)
	: FilterWheel(filterwheelname(camera)), _camera(camera) {
}

unsigned int	AtikFilterwheel::nFilters() {
	unsigned int	filtercount;
	bool	moving;
	unsigned int	current;
	unsigned int	target;
	_camera->getFilterWheelStatus(&filtercount, &moving, &current, &target);
	return filtercount;
}

unsigned int	AtikFilterwheel::currentPosition() {
	unsigned int	filtercount;
	bool	moving;
	unsigned int	current;
	unsigned int	target;
	_camera->getFilterWheelStatus(&filtercount, &moving, &current, &target);
	return current;
}

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

void	AtikFilterwheel::select(size_t filterindex) {
	_camera->setFilter(filterindex);
}

} // namespace atik
} // namespace camera
} // namespace stro
