/*
 * Guiders.cpp -- guider reference repository implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guiders.h>
#include <AstroDebug.h>
#include <guider.hh>
#include <OrbSingleton.h>
#include <DeviceMap.h>
#include <CorbaExceptionReporter.h>
#include <AstroDevice.h>
#include <stdexcept>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Class of internals for the guiders
//////////////////////////////////////////////////////////////////////

/**
 * \brief internals class for Guider repository
 */
class Guider_internals : public DeviceMap<Astro::Guider> {
public:
	Guider_internals() { }
	virtual void	assign(const std::string& guiderid,
				const std::vector<std::string>& arguments);
};

/**
 * \brief assign a guider to a name
 */
void	Guider_internals::assign(const std::string& guiderid,
		const std::vector<std::string>& arguments) {

	if (arguments.size() < 5) {
		throw devicemap_error("guider assign needs 5 arguments");
	}
	std::string	cameraname = arguments[2];
	long	ccdno = stol(arguments[3]);
	std::string	guiderportname = arguments[4];
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"guider(camera=%s, ccd=%ld, guiderport=%s)",
		cameraname.c_str(), ccdno, guiderportname.c_str());

	// create a GuiderDescriptor
	Astro::GuiderFactory::GuiderDescriptor	*descriptor
		= new Astro::GuiderFactory::GuiderDescriptor();
	descriptor->cameraname = CORBA::string_dup(cameraname.c_str());
	descriptor->ccdid = ccdno;
	descriptor->guiderportname = CORBA::string_dup(guiderportname.c_str());
	Astro::GuiderFactory::GuiderDescriptor_var	descvar = descriptor;

	// geht the modules interface
	Astro::OrbSingleton	orb;
	Astro::GuiderFactory_var	guiderfactory;
	try {
		guiderfactory = orb.getGuiderfactory();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x).c_str();
		debug(LOG_ERR, DEBUG_LOG, 0, "getGuiderfactory() exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	Astro::Guider_ptr	guider = guiderfactory->get(*descriptor);

	// assign the Guider_var object to this 
	DeviceMap<Astro::Guider>::assign(guiderid, guider);
}


//////////////////////////////////////////////////////////////////////
// Guiders implementation
//////////////////////////////////////////////////////////////////////

Guider_internals	*Guiders::internals = NULL;

/**
 * \brief create the Guiders object
 */
Guiders::Guiders() {
	if (NULL == internals) {
		internals = new Guider_internals();
	}
}

GuiderWrapper	Guiders::byname(const std::string& guiderid) {
	return internals->byname(guiderid);
}

void	Guiders::release(const std::string& guiderid) {
	internals->release(guiderid);
}

void	Guiders::assign(const std::string& guiderid,
			const std::vector<std::string>& arguments) {
	internals->assign(guiderid, arguments);
}

} // namespace cli
} // namespace astro
