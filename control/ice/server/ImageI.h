/*
 * ImageI.h -- Image servant declaration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageI_h
#define _ImageI_h

#include <image.h>
#include <AstroImage.h>
#include <typeindex>

namespace snowstar {

class ImageI : virtual public Image {
protected:
	astro::image::ImagePtr	_image;
private:
	std::string	_filename;
	ImagePoint	_origin;
	ImageSize	_size;
protected:
	std::type_index	_type;
	int	_bytesperpixel;
	int	_bytespervalue;
	int	_planes;
public:
	ImageI(astro::image::ImagePtr image, const std::string& filename);
	virtual ~ImageI();
	virtual std::string	name(const Ice::Current& current);
	virtual int	age(const Ice::Current& current);
	virtual ImageSize	size(const Ice::Current& current);
	virtual ImagePoint	origin(const Ice::Current& current);
	virtual int	bytesPerPixel(const Ice::Current& current);
	virtual int	planes(const Ice::Current& current);
	virtual int	bytesPerValue(const Ice::Current& current);
	virtual bool	hasMeta(const std::string& keyword,
				const Ice::Current& current);
	virtual Metavalue	getMeta(const std::string& keyword,
					const Ice::Current& current);
	virtual void	setMetavalue(const Metavalue& metavalue,
				const Ice::Current& current);
	virtual void	setMetadata(const Metadata& metadata,
				const Ice::Current& current);
	virtual ImageFile	file(const Ice::Current& current);
	virtual int	filesize(const Ice::Current& current);
	virtual void	toRepository(const std::string& reponame,
				const Ice::Current& current);
	virtual void	remove(const Ice::Current& current);
	ImagePrx	createProxy(const std::string& filename,
				const Ice::Current& current);
};

class ByteImageI : virtual public ByteImage, virtual public ImageI {
public:
	ByteImageI(astro::image::ImagePtr image, const std::string& filename);
	virtual ~ByteImageI();
	virtual ByteSequence	getBytes(const Ice::Current& current);
};

class ShortImageI : virtual public ShortImage, virtual public ImageI {
public:
	ShortImageI(astro::image::ImagePtr image, const std::string& filename);
	virtual ~ShortImageI();
	virtual ShortSequence	getShorts(const Ice::Current& current);
};

class IntImageI : virtual public IntImage, virtual public ImageI {
public:
	IntImageI(astro::image::ImagePtr image, const std::string& filename);
	virtual ~IntImageI();
	virtual IntSequence	getInts(const Ice::Current& current);
};

class FloatImageI : virtual public FloatImage, virtual public ImageI {
public:
	FloatImageI(astro::image::ImagePtr image, const std::string& filename);
	virtual ~FloatImageI();
	virtual FloatSequence	getFloats(const Ice::Current& current);
};

class DoubleImageI : virtual public DoubleImage, virtual public ImageI {
public:
	DoubleImageI(astro::image::ImagePtr image, const std::string& filename);
	virtual ~DoubleImageI();
	virtual DoubleSequence	getDoubles(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _ImageI_h */
