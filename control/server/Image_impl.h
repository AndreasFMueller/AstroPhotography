/*
 * Image_impl.h -- CORBA Image wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Image_impl_h
#define _Image_impl_h

#include "../idl/device.hh"
#include <AstroImage.h>

namespace Astro {

class Image_impl : public POA_Astro::Image {
	astro::image::ImagePtr	_image;
public:
	inline	Image_impl(astro::image::ImagePtr image) : _image(image) { }
	virtual Astro::ImagePoint	origin();
	virtual Astro::ImageSize	size();
	virtual void	write(const char *filename, ::CORBA::Boolean overwrite);
};

} // namespace Astro

#endif /* _Image_impl_h */
