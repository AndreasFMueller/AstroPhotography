/*
 * Mock1Camera.cpp -- mock1 camera
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include "Mock1Camera.h"
#include "Mock1Ccd.h"
#include <AstroFormat.h>
#include <iostream>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace mock1 {

Mock1Camera::Mock1Camera(int _id) : id(_id) {
	_name = stringprintf("mock1:%d", _id);

	CcdInfo	ccd0("primary ccd", ImageSize(1024, 768), 0);
	ccd0.addMode(Binning(1,1));
	ccd0.pixelwidth(0.00001);
	ccd0.pixelheight(0.00001);
	ccdinfo.push_back(ccd0);

	CcdInfo	ccd1("secondary ccd", ImageSize(640, 480), 1);
	ccd1.addMode(Binning(1,1));
	ccd1.pixelwidth(0.00001);
	ccd1.pixelheight(0.00001);
	ccdinfo.push_back(ccd1);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "mock1 has %d ccds", this->nCcds());
}

CcdPtr	Mock1Camera::getCcd0(size_t id) {
	if (id > 1) {
		throw std::runtime_error("bad ccd id");
	}
	return CcdPtr(new Mock1Ccd(ccdinfo[id], this->id, id));
}

} // namespace mock1
} // namespace camera
} // namespace astro
