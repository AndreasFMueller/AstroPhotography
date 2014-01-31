/*
 * Filterwheels.cpp -- filterwheel reference repository implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Cameras.h>
#include <Filterwheels.h>
#include <DeviceMap.h>
#include <AstroDebug.h>
#include <CorbaExceptionReporter.h>
#include <camera.hh>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Filterwheel_internals implementation
//////////////////////////////////////////////////////////////////////

class Filterwheel_internals : public DeviceMap<Astro::FilterWheel> {
public:
	Filterwheel_internals() { }
	virtual void	assign(const std::string& filterwheelid,
				const std::vector<std::string>& arguments);
};

void	Filterwheel_internals::assign(const std::string& filterwheelid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assigning filterwheel of name %s",
		filterwheelid.c_str());

	// make sure we have enough arguments
	if (arguments.size() < 3) {
		throw devicemap_error("filterwheel assign needs 3 arguments");
	}

	// first locate the camera specified by the arguments
	std::string	cameraid = arguments[2];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get filterwheel from camera %s",
		cameraid.c_str());

	// first try to get the camera with that id
	Cameras	cameras;
	CameraWrapper	camera = cameras.byname(cameraid);

	// then check whether it has enough ccds
	if (!camera->hasFilterWheel()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera %s has no filter wheel",
			camera->getName());
		throw devicemap_error("camera has no filterwheel");
	}

	// get the ccd from the camera
	Astro::FilterWheel_ptr	filterwheel;
	try {
		filterwheel = camera->getFilterWheel();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getFilterWheel exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	if (CORBA::is_nil(filterwheel)) {
		throw devicemap_error("could not get filterwheel");
	}

	// store it in the device map
	DeviceMap<Astro::FilterWheel>::assign(filterwheelid, filterwheel);
}

//////////////////////////////////////////////////////////////////////
// Filterwheels implementation
//////////////////////////////////////////////////////////////////////

Filterwheel_internals	*Filterwheels::internals = NULL;

Filterwheels::Filterwheels() {
	if (NULL == internals) {
		internals = new Filterwheel_internals();
	}
}

FilterwheelWrapper	Filterwheels::byname(const std::string& filterwheelid) {
	return internals->byname(filterwheelid);
}

void	Filterwheels::assign(const std::string& filterwheelid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assign");
	internals->assign(filterwheelid, arguments);
}

void	Filterwheels::release(const std::string& filterwheelid) {
	internals->release(filterwheelid);
}

} // namespace cli
} // namespace astro
