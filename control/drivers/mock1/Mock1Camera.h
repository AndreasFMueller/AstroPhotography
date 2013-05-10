/*
 * Mock1Camera.h -- mock1 camera
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _Mock1Camera_h
#define _Mock1Camera_h

#include <AstroCamera.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace mock1 {

class Mock1Camera : public Camera {
	int	id;
public:
	Mock1Camera(int _id);
	virtual	~Mock1Camera() { }
	virtual CcdPtr	getCcd(size_t id);
};

} // namespace mock1
} // namespace camera
} // namespace astro

#endif /* _Mock1Camera_h */
