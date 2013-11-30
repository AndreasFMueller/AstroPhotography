/*
 * Images_impl.h -- servant for images in a directory
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Image_impl_h
#define _Image_impl_h

#include <image.hh>
#include <string>
#include <ImageDirectory.h>

namespace Astro {

/**
 * \brief Images implementation
 *
 * The Images service gives access to a directory containing FITS images.
 * The methods allow to retrieve a list of available files and to read
 * basic information about each file. The getImage method allows then to
 * get a reference to such an image.
 */
class Images_impl : public virtual POA_Astro::Images, public ImageDirectory {
	std::string	fullname(const std::string& filename);
public:
	Images_impl();
	virtual CORBA::Long	imageSize(const char *name);
	virtual CORBA::Long	imageAge(const char *name);
	virtual Astro::Images::ImageList* listImages();
	virtual Image_ptr getImage(const char* name);
};

} // namespace Astro

#endif /* _Image_impl_h */
