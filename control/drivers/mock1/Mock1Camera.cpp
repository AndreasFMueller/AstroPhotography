/*
 * Mock1Camera.cpp -- mock1 camera
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include "Mock1Camera.h"
#include "Mock1Ccd.h"
#include <iostream>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace mock1 {

CcdPtr	Mock1Camera::getCcd(int id) {
	if ((id < 0) || (id > 1)) {
		throw std::runtime_error("bad ccd id");
	}
	ImageSize	size(100 + 40 * id, 50 + 30 * id);
	return CcdPtr(new Mock1Ccd(size, this->id, id));
}

} // namespace mock1
} // namespace camera
} // namespace astro
