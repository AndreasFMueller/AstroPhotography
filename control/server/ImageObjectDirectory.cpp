/*
 * ImageObjectDirectory.cpp -- directory containing images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageObjectDirectory.h>
#include <AstroDebug.h>
#include <OrbSingleton.h>
#include <unistd.h>
#include <AstroIO.h>
#include <string.h>
#include <errno.h>
#include <image.hh>

namespace Astro {

/**
 * \brief Get an image object reference
 */
Image_ptr	ImageObjectDirectory::getImage(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering object id for %s",
		filename.c_str());
	// check whether the file exists
	if (!isFile(filename)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image %s does not exist",
			filename.c_str());
		NotFound	notfound;
		notfound.cause = CORBA::string_dup("file does not exist");
		throw notfound;
	}

	// create an object id associated with the file name
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId(filename.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "oid created");
	
	// now create an object reference in the POA for images
	OrbSingleton	orb;
	PoaName	poapath("Images");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting POA for Images");
	PortableServer::POA_var	images_poa = orb.findPOA(poapath);
	CORBA::Object_var	obj
		= images_poa->create_reference_with_id(oid,
			"IDL:/Astro/Image");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reference for image created");
	return Image::_narrow(obj);
}

} // namespace Astro
