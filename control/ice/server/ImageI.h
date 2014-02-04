/*
 * ImageI.h -- Image servant declaration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageI_h
#define _ImageI_h

#include <image.h>
#include <AstroImage.h>
#include <ImageDirectory.h>

namespace snowstar {

class ImageI : virtual public Image {
	astro::image::ImageDirectory&	_imagedirectory;
protected:
	astro::image::ImagePtr	_image;
private:
	std::string	_filename;
	ImagePoint	_origin;
	ImageSize	_size;
protected:
	int	_bytesperpixel;
private:
	int	_bytespervalue;
	int	_planes;
public:
	ImageI(astro::image::ImageDirectory& imagedirectrory,
		astro::image::ImagePtr image, const std::string& filename);
	virtual ~ImageI();
	virtual ImageSize	size(const Ice::Current& current);
	virtual ImagePoint	origin(const Ice::Current& current);
	virtual int	bytesPerPixel(const Ice::Current& current);
	virtual int	planes(const Ice::Current& current);
	virtual int	bytesPerValue(const Ice::Current& current);
	virtual ImageFile	file(const Ice::Current& current);
	virtual int	filesize(const Ice::Current& current);
	virtual void	remove(const Ice::Current& current);
	ImagePrx	createProxy(const std::string& filename,
				const Ice::Current& current);
};

class ByteImageI : virtual public ByteImage, virtual public ImageI {
public:
	ByteImageI(astro::image::ImageDirectory& imagedirectrory,
		astro::image::ImagePtr image, const std::string& filename);
	virtual ~ByteImageI();
	virtual ByteSequence	getBytes(const Ice::Current& current);
};

class ShortImageI : virtual public ShortImage, virtual public ImageI {
public:
	ShortImageI(astro::image::ImageDirectory& imagedirectrory,
		astro::image::ImagePtr image, const std::string& filename);
	virtual ~ShortImageI();
	virtual ShortSequence	getShorts(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _ImageI_h */
