/*
 * Ccds.cpp -- ccd reference repository implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ccds.h>
#include <DeviceMap.h>
#include <AstroDebug.h>
#include <Cameras.h>
#include <CorbaExceptionReporter.h>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Ccd_internals implementation
//////////////////////////////////////////////////////////////////////

class Ccd_internals : public DeviceMap<Astro::Ccd> {
public:
	Ccd_internals() { }
	virtual void	assign(const std::string& ccdid,
				const std::vector<std::string>& arguments);
};

void	Ccd_internals::assign(const std::string& ccdid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assigning ccd of name %s",
		ccdid.c_str());

	// make sure we have enough arguments
	if (arguments.size() < 4) {
		throw devicemap_error("ccd assign needs 4 arguments");
	}

	// first locate the camera specified by the arguments
	std::string	cameraid = arguments[2];
	std::string	ccdnumber = arguments[3];
	int	ccdno = stoi(ccdnumber);
	if (ccdno < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd number is negative");
		throw devicemap_error("negative ccd number");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ccd %d from camera %s",
		ccdno, cameraid.c_str());

	// first try to get the camera with that id
	Cameras	cameras;
	CameraWrapper	camera = cameras.byname(cameraid);

	// then check whether it has enough ccds
	if (ccdno >= camera->nCcds()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd number %d out of range %d",
			ccdno, camera->nCcds());
		throw devicemap_error("ccd number out of range");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd number is valid");

	// get the ccd from the camera
	Astro::Ccd_ptr	ccd;
	try {
		ccd = camera->getCcd(ccdno);
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getCcd exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	if (CORBA::is_nil(ccd)) {
		throw devicemap_error("could not get camera");
	}

	// store it in the device map
	DeviceMap<Astro::Ccd>::assign(ccdid, ccd);
}

//////////////////////////////////////////////////////////////////////
// Ccds implementation
//////////////////////////////////////////////////////////////////////

Ccd_internals	*Ccds::internals = NULL;

Ccds::Ccds() {
	if (NULL == internals) {
		internals = new Ccd_internals();
	}
}

CcdWrapper	Ccds::byname(const std::string& ccdid) {
	return internals->byname(ccdid);
}

void	Ccds::assign(const std::string& ccdid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assign");
	internals->assign(ccdid, arguments);
}

void	Ccds::release(const std::string& ccdid) {
	internals->release(ccdid);
}

} // namespace cli
} // namespace astro
