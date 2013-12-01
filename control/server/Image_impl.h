/*
 * Image_impl.h -- CORBA Image wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Image_impl_h
#define _Image_impl_h

#include <image.hh>
#include <AstroImage.h>
#include <AstroFormat.h>
#include <ImageObjectDirectory.h>

namespace Astro {

/**
 * \brief Image servant definition
 */
class Image_impl : public virtual POA_Astro::Image, public ImageObjectDirectory {
	std::string	_filename;
	void	setup(astro::image::ImagePtr image);
public:
	// constructors
	Image_impl(const std::string& filename);

	// static fields
private:
	Astro::ImagePoint	_origin;
public:
	virtual Astro::ImagePoint	origin() { return _origin; }

private:
	Astro::ImageSize	_size;
public:
	virtual Astro::ImageSize	size() { return _size; }

private:
	int	_bytesperpixel;
public:
	virtual CORBA::Long	bytesPerPixel() { return _bytesperpixel; }

private:
	int	_bytespervalue;
public:
	virtual CORBA::Long	bytesPerValue() { return _bytespervalue; }

private:
	int	_planes;
public:
	virtual CORBA::Long	planes() { return _planes; }

	// access to the image file data
	virtual Astro::Image::ImageFile	*file();
	virtual CORBA::Long	filesize();

	virtual void	remove();
protected:
	astro::image::ImagePtr	getImage();
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
	ByteImage_impl(const std::string& filename)
		: Image_impl(filename) { }
	virtual ~ByteImage_impl();
	Astro::ByteImage::ByteSequence	*getBytes();
};

/**
 * \brief ShortImage servant definition
 */
class ShortImage_impl : public Image_impl, public POA_Astro::ShortImage {
public:
	ShortImage_impl(const std::string& filename)
		: Image_impl(filename) { }
	virtual ~ShortImage_impl();
	Astro::ShortImage::ShortSequence	*getShorts();
};

} // namespace Astro

#endif /* _Image_impl_h */
