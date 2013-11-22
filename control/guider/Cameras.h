/*
 * Cameras.h -- repository of camera references
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Cameras_h
#define _Cameras_h

#include <AstroDebug.h>
#include <camera.hh>
#include <ObjWrapper.h>
#include <vector>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Camera>	CameraWrapper;

class Camera_internals;

/**
 * \brief class to mediate access to camera references by short name
 */
class Cameras {
	static Camera_internals	*internals;
public:
	Cameras();
	CameraWrapper	byname(const std::string& cameraid);
	void	release(const std::string& cameraid);
	void	assign(const std::string& cameraid,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _Cameras_h */
