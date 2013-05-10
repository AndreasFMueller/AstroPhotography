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

Mock1Camera::Mock1Camera(int _id) : id(_id) {
	CcdInfo	ccd0;
	ccd0.size = ImageSize(1024, 768);
	ccd0.name = "primary ccd";
	ccd0.binningmodes.push_back(Binning(1,1));
	ccdinfo.push_back(ccd0);

	CcdInfo	ccd1;
	ccd1.size = ImageSize(640, 480);
	ccd1.name = "secondary ccd";
	ccd1.binningmodes.push_back(Binning(1,1));
	ccdinfo.push_back(ccd1);
}

CcdPtr	Mock1Camera::getCcd(size_t id) {
	if ((id < 0) || (id > 1)) {
		throw std::runtime_error("bad ccd id");
	}
	return CcdPtr(new Mock1Ccd(ccdinfo[id], this->id, id));
}

} // namespace mock1
} // namespace camera
} // namespace astro
