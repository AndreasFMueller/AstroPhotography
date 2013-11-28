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
