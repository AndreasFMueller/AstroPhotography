/*
 * Image_impl.h -- CORBA Image wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Image_impl_h
#define _Image_impl_h

#include <device.hh>
#include <AstroImage.h>
#include <AstroFormat.h>

namespace Astro {

/**
 * \brief Image servant definition
 */
class Image_impl : public virtual POA_Astro::Image {
protected:
	astro::image::ImagePtr	_image;
public:
	inline	Image_impl(astro::image::ImagePtr image) : _image(image) { }
	virtual Astro::ImagePoint	origin();
	virtual Astro::ImageSize	size();
	virtual char	*write(const char *filename, ::CORBA::Boolean overwrite);
	virtual CORBA::Long	bytesPerPixel();
	virtual CORBA::Long	bytesPerValue();
	virtual CORBA::Long	planes();
	virtual CORBA::Double	max();
	virtual CORBA::Double	min();
	virtual CORBA::Double	mean();
	virtual CORBA::Double	median();
	virtual Astro::Image::ImageFile	*file();
};

/*
 * Note: the multiple inheritance (from POA_Astro::ByteImage and from
 * Image_impl) is necessary so that POA actuall recognizes the return
 * value as a ByteImage. If one only Inherits from Image_impl, then the
 * objects are presented to the client as of class Image, not of the
 * derived class.
 */

/**
 * \brief ByteImage servant definition
 */
class ByteImage_impl : public Image_impl, public POA_Astro::ByteImage {
public:
	inline ByteImage_impl(astro::image::ImagePtr image)
		: Image_impl(image) { }
	Astro::ByteImage::ByteSequence	*getBytes();
};

/**
 * \brief ShortImage servant definition
 */
class ShortImage_impl : public Image_impl, public POA_Astro::ShortImage {
public:
	inline ShortImage_impl(astro::image::ImagePtr image)
		: Image_impl(image) { }
	Astro::ShortImage::ShortSequence	*getShorts();
};

} // namespace Astro

#endif /* _Image_impl_h */
