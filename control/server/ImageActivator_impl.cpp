/*
 * ImageActivator_impl.cpp -- Activator to activate Image servcants
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageActivator_impl.h>
#include <Image_impl.h>
#include <sys/stat.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroFilterfunc.h>

namespace Astro {

/**
 * \brief Incarnate a servant for an image
 */
PortableServer::Servant	ImageActivator_impl::incarnate(
				const PortableServer::ObjectId& oid,
				PortableServer::POA_ptr poa
	) throw (CORBA::SystemException, PortableServer::ForwardRequest) {
	// the object id encodes the file name, so first have to convert
	// the object id into a file name
	std::string	filename;
	try {
		filename = PortableServer::ObjectId_to_string(oid);
	} catch (const CORBA::BAD_PARAM&) {
		throw CORBA::OBJECT_NOT_EXIST();
	}
	std::string	fn = fullname(filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct servant from file name: %s",
		fn.c_str());

	// find out whether this file really exists
	if (!isFile(filename)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not a regular file: %s",
			fn.c_str());
		throw CORBA::OBJECT_NOT_EXIST();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is a regular file",
		fn.c_str());

	// read the image
	astro::io::FITSin	infile(fn);
	ImagePtr	image;
	try {
		image = infile.read();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"could not read file %s: %s", fn.c_str(), x.what());
		throw CORBA::OBJECT_NOT_EXIST();
	}

	// build a servant of approriate type
	try {
		switch (astro::image::filter::bytespervalue(image)) {
		case 1:
			return new Astro::ByteImage_impl(filename);
			break;
		case 2:
			return new Astro::ShortImage_impl(filename);
			break;
		default:
			debug(LOG_ERR, DEBUG_LOG, 0,
				"image type we cannot handle: %d bytes per value",
				astro::image::filter::bytespervalue(image));
		}
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"exception while creating servant: %s", x.what());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"exception while creating servant");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create image servant %s",
		fn.c_str());
	throw CORBA::OBJECT_NOT_EXIST();
}

/**
 * \brief Etherealize a servant
 */
void	ImageActivator_impl::etherealize(
		const PortableServer::ObjectId&	oid,
		PortableServer::POA_ptr		poa,
		PortableServer::Servant		serv,
		CORBA::Boolean			cleanup_in_progress,
		CORBA::Boolean			remaining_activations
	) throw (CORBA::SystemException) {

	// get the file name
	std::string	filename;
        try {
		filename = PortableServer::ObjectId_to_string(oid);
        } catch (const CORBA::BAD_PARAM&) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "have not filename");
        }

	// when there are no remaining activations, remove the image
	// from the image directory
	if (remaining_activations) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remaining activations");
		return;
	}
		
	// remove the servant
	delete serv;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "servant deleted");

	// remove the image from the ImageDirectory
	try {
		ImageDatabaseDirectory::remove(filename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s deleted", filename.c_str());
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during etherialize: %s",
			x.what());
	}
}

} // namespace Astro
