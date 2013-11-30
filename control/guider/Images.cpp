/*
 * Images.cpp -- image reference repository implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Images.h>
#include <DeviceMap.h>
#include <AstroDebug.h>
#include <Cameras.h>
#include <CorbaExceptionReporter.h>
#include <OrbSingleton.h>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Image_internals implementation
//////////////////////////////////////////////////////////////////////

class Image_internals : public DeviceMap<Astro::Image> {
public:
	Image_internals() { }
	virtual void	assign(const std::string& imageid,
				const std::vector<std::string>& arguments);
	void	assign(const std::string& imageid, Astro::Image_ptr image);
};

void	Image_internals::assign(const std::string& imageid,
		Astro::Image_ptr image) {
	DeviceMap<Astro::Image>::assign(imageid, image);
}

void	Image_internals::assign(const std::string& imageid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assigning image of name %s",
		imageid.c_str());

	// make sure we have enough arguments
	if (arguments.size() < 3) {
		throw devicemap_error("image assign needs 3 arguments");
	}

	// first locate the camera specified by the arguments
	std::string	imagefilename = arguments[2];

	// get the orb, 
	Astro::OrbSingleton	orb;
	Astro::Images_var	images;
	try {
		images = orb.getImages();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getImages() exception %s",
			s.c_str());
		throw std::runtime_error(s);
	}

	// get the image reference
	try {
		assign(imageid, images->getImage(imagefilename.c_str()));
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getImage exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
}

//////////////////////////////////////////////////////////////////////
// Images implementation
//////////////////////////////////////////////////////////////////////

Image_internals	*Images::internals = NULL;

Images::Images() {
	if (NULL == internals) {
		internals = new Image_internals();
	}
}

ImageWrapper	Images::byname(const std::string& imageid) {
	return internals->byname(imageid);
}

void	Images::assign(const std::string& imageid,
		Astro::Image_ptr image) {
	internals->assign(imageid, image);
}

void	Images::assign(const std::string& imageid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assign");
	internals->assign(imageid, arguments);
}

void	Images::release(const std::string& imageid) {
	internals->release(imageid);
}

} // namespace cli
} // namespace astro
