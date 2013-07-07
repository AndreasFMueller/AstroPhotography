/*
 * AstroImage.h -- abstraction for raw images received from cameras
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroImage_h
#define _AstroImage_h

#include <stdexcept>
#include <tr1/memory>
#include <algorithm>
#include <iostream>
#include <AstroPixel.h>
#include <limits>
#include <vector>

namespace astro {
namespace image {

/**
 * \brief Abstraction for points within an image
 *
 * Contrary to the usualy convention in computer graphics that the origin
 * of a picture is in the upper right corner, ImagePoint instances referene
 * points of an Image with the origin at the lower left corner. This is
 * the more reasonable convention in the astrophotography setting, because
 * the FITS files commonly used in Astrophotography follow the same
 * convention.
 */
class ImagePoint {
public:
	unsigned int	x, y;
	ImagePoint(unsigned int _x = 0, unsigned int _y = 0) : x(_x), y(_y) { }
	bool	operator==(const ImagePoint& other) const;
	ImagePoint	operator+(const ImagePoint& other) const;
	ImagePoint	operator-(const ImagePoint& other) const;
	std::string	toString() const;
};

class ImageRectangle;
/**
 * \brief Size of an image or rectangle
 *
 * in the size object we declare all members const, so that users cannot
 * change them destroy the consistency we want to enforce between the
 * number of pixels and width/height.
 */
class ImageSize {
public:
	unsigned int	width, height;
	unsigned int	pixels;
	ImageSize(unsigned int _width = 0, unsigned int _height = 0);
	bool	operator==(const ImageSize& other) const;
	bool	operator!=(const ImageSize& other) const;
	bool	bounds(const ImagePoint& point) const;
	bool	bounds(const ImageRectangle& rectangle) const;
	bool	contains(const ImagePoint& point) const;
	bool	contains(unsigned int x, unsigned int y) const;
	int	chi(unsigned int x, unsigned int y) const;
	std::string	toString() const;
};

/**
 * \brief Rectangle
 *
 * The ImageRectangle abstraction is used to specify rectangles within an
 * image. An ImageRectangle is specified by a ImagePoint, the origin, which
 * is the lower left corner of the rectangle, and a an ImageSize, which
 * specifies width and height of the rectangle,
 */
class ImageRectangle {
public:
	ImagePoint	origin;
	ImageSize		size;
	bool	contains(const ImagePoint& point) const;
	bool	contains(const ImageRectangle& rectangle) const;
	ImageRectangle(unsigned int w = 0, unsigned int h = 0) : size(w, h) { }
	ImageRectangle(const ImageSize& _size) : origin(0, 0), size(_size) { }
	ImageRectangle(const ImagePoint& _origin, const ImageSize& _size)
		: origin(_origin), size(_size) { }
	ImageRectangle(const ImageRectangle& rectangle,
		const ImagePoint& translatedby);
	ImageRectangle(const ImageRectangle& rectangle,
		const ImageRectangle& subrectangle);
	bool	operator==(const ImageRectangle& other) const;
	const ImagePoint&	lowerLeftCorner() const;
	ImagePoint	lowerRightCorner() const;
	ImagePoint	upperRightCorner() const;
	ImagePoint	upperLeftCorner() const;
	std::string	toString() const;
};

/**
 * \brief Image base class
 *
 * Images in Astrophotographe can have wildly varying pixel types,
 * and it does not make sense to always convert to a common class.
 * E.g. there are cameras with very large CCDs in the 16MPixel range
 * with each Pixel requiring an unsigned short for encoding. Such a
 * camera delivers an Image of about 32MB. On the other hand, there
 * are small cameras with only 640 x 480 pixels and 8bit per pixel.
 *
 * The base class thus only handles arithmetic of computing offsets
 * into an array of pixel values, which is addressed line by line.
 * The lower left corner with coordinate (0,0) has pixel offset 0,
 * the lower right corner with coordinates (width - 1, 0) has pixel
 * offset width-1. The pixel with pixeloffset width has image coordinates
 * (0,1).
 *
 * An image can also have a mosaic type. Mosaic types are mainly useful
 * for Bayer Matrix images, and the library provides some methods
 * to convert images with nontrivial Bayer matrix into images with RGB
 * pixels.
 *
 * Images have immutable size and cannot be assigned, which makes them
 * quite awkward to use. But because of the large data sets involved,
 * a smart pointer has to be used anyway. Such smart pointers are
 * defined for the Image template class that inherits from ImageBase.
 */
class ImageBase {
public:
	/**
	 * \brief Constants describing pixel layout in bayer matrix.
	 *
	 * The four BAYER_ constants indicate the position of the red
	 * pixel. The last two bits can be interpreted as the coordinates
	 * of the red pixel in a 2x2 square of the Bayer matrix. The last
	 * bit is the x-coordinate, the last but one bit is the y-coordinate.
	 * so the constant 2 has last bit 0 and last but one bit 1, translating
	 * into a bayer matrix that has the red pixel in the second row and
	 * the first column, i.e. in the lower left corner
	 */
	typedef enum mosaic_e {
		NONE = 0,
		BAYER_RGGB = 4,
		BAYER_GRBG = 5,
		BAYER_GBRG = 6,
		BAYER_BGGR = 7
	} mosaic_type;
	mosaic_type	mosaic;

	// the size is publicly accessible, but cannot be changed
	ImageSize	size;

	// constructors and destructor
	ImageBase(unsigned int w = 0, unsigned int h = 0);
	ImageBase(const ImageSize& _size);
	ImageBase(const ImageRectangle& _frame);
	ImageBase(const ImageBase& other);
	virtual	~ImageBase() { }

	// comparison and pixel offset computation
	virtual bool	operator==(const ImageBase& other) const;
	virtual unsigned int	pixeloffset(unsigned int x, unsigned int y) const;
	virtual unsigned int	pixeloffset(const ImagePoint& p) const;

	// methods related to the mosaic stuff
	bool	isR(unsigned int x, unsigned int y) const;
	bool	isG(unsigned int x, unsigned int y) const;
	bool	isB(unsigned int x, unsigned int y) const;
	bool	isGr(unsigned int x, unsigned int y) const;
	bool	isGb(unsigned int x, unsigned int y) const;

	virtual unsigned int bitsPerPixel() const { return 0; }
	unsigned int bytesPerPixel() const;
};

/**
 * \brief Iterators for Images
 *
 * Many operations on images apply to all pixels, so we need a fast
 * way to loop through all pixels of a line, a row or even a complete
 * image. There is no problem to loop through the pixels in an array
 * of pixel values, asn pointers are natural iterators anyway, but
 * To iterate through line or column we construct a set of iterators.
 * ImageIteratorBase is the base class for these iterators.
 *
 * The stride attribute of the iterator is used to implement iterators
 * that iterate through rows (stride = 1) or columns (stride = width).
 */
// base iterator classes
class ImageIteratorBase {
protected:
	unsigned int	first;	// first index, always >= 0, the end is
			// indicated by setting it to
			// std::numeric_limits<unsigned int>::max(),
			// the iterator then points nowhere (NULL pointer)
	unsigned int	last;	// last index
	unsigned int	offset; // 
	unsigned int	stride;
public:
	ImageIteratorBase(unsigned int _first, unsigned int _last,
		unsigned int _offset, unsigned int _stride) :
		first(_first), last(_last), offset(_offset), stride(_stride) { }
	ImageIteratorBase(unsigned int _first, unsigned int _last,
		unsigned int _stride = std::numeric_limits<unsigned int>::max())
		: first(_first), last(_last), offset(_first), stride(_stride) { }
	ImageIteratorBase() : first(0), last(0), offset(std::numeric_limits<unsigned int>::max()), stride(1) { }
	ImageIteratorBase&	operator++();
	ImageIteratorBase&	operator--();
	ImageIteratorBase	operator++(int);
	ImageIteratorBase	operator--(int);
	ImageIteratorBase	operator+(const int steps) const;
	ImageIteratorBase	operator-(const int steps) const;
	bool	valid() const;
	bool	invalid() const;
	// image iterators are equal, if they point to the same pixel
	bool	operator==(const ImageIteratorBase& other) const;
	bool	operator!=(const ImageIteratorBase& other) const;
	unsigned int	pixeloffset() const throw(std::range_error);
	unsigned int	f() const { return first; }
	unsigned int	l() const { return last; }
};

/**
 * \brief Abstraction for rows and columns
 *
 * When constructing an iterator from an image, one has to specify 
 * whether the iterator is for a row or a column, and has to adapt
 * the stride correspondingly. The ImageLine class is the common
 * base class for rows and columns. Given an ImageLine object,
 * it is easy to get an iterator that iterates throught that line.
 */
class	ImageLine {
public:
	const	unsigned int firstoffset, lastoffset;
	const	unsigned int stride;
protected:
	ImageLine(unsigned int _firstoffset, unsigned int _lastoffset, unsigned int _stride)
		: firstoffset(_firstoffset), lastoffset(_lastoffset),
		  stride(_stride) { }
public:
	ImageIteratorBase	begin() const;
	ImageIteratorBase	end() const;
};

/**
 * \brief Base class for row iterators
 */
class	ImageRow : public ImageLine {
public:
	const unsigned int	y;
	ImageRow(const ImageSize size, unsigned int _y)
		: ImageLine(size.width * _y, size.width * (_y + 1) - 1, 1),
		  y(_y) { }
};

/**
 * \brief Base class for column iterators
 */
class	ImageColumn : public ImageLine {
public:
	const unsigned int	x;
	ImageColumn(const ImageSize& size, unsigned int _x)
		: ImageLine(_x, _x + size.pixels - size.width, size.width),
		  x(_x) { }
};

/**
 * \brief Image class
 *
 * The Image template class implements images with different pixel types
 * as specified by the template argument. Images have an immutable size
 */
template<typename Pixel>
class Image : public ImageBase {
public:
	/**
	 * \brief	Array containing the pixel values
	 */
	Pixel	*pixels;

	/**
	 * \brief	Create a new Image
	 * 
	 * creates a new image of a given size and optionally with an array
	 * of pixel values. If the pixel values are not specified, a
	 * new array of undefined contents is allocated.
	 *
 	 * \param w	width
	 * \param h	height
	 * \param p	preexisting array of pixels. If specified, the
	 *		new Image instance will take ownership of the
	 *		supplied Pixel array and will free it when the
	 *		Image is deallocated.
	 */
	Image<Pixel>(unsigned int _w, unsigned int _h, Pixel *p = NULL)
		: ImageBase(_w, _h) {
		if (p) {
			pixels = p;
		} else {
			pixels = new Pixel[size.pixels];
		}
	}

	/**
	 * \brief	Create a new Image
	 * 
	 * creates a new image of a given size and optionally with an array
	 * of pixel values. If the pixel values are not specified, a
	 * new array of undefined contents is allocated.
	 *
 	 * \param size	image size
	 * \param p	preexisting array of pixels. If specified, the
	 *		new Image instance will take ownership of the
	 *		supplied Pixel array and will free it when the
	 *		Image is deallocated.
	 */
	Image<Pixel>(const ImageSize& size, Pixel *p = NULL) : ImageBase(size) {
		if (p) {
			pixels = p;
		} else {
			pixels = new Pixel[size.pixels];
		}
	}

	/**
	 * \brief	Copy an image from a different pixel type
	 *
	 * \param
 	 */
	template<typename srcPixel>
	Image<Pixel>(const Image<srcPixel>& other) : ImageBase(other.size) {
		pixels = new Pixel[size.pixels];
		convertPixelArray(pixels, other.pixels, size.pixels);
	}

	/**
	 * \brief	Extract a subimage from an image
	 */
	Image<Pixel>(const Image<Pixel>& other, const ImageRectangle& frame);

	/**
	 * \brief Copy an image
	 *
	 * Copying an image could be done using the subimage constructor,
	 * but copying a complete image can be implemented more efficiently,
	 * because the whole pixel array and not only some rows of  it
	 * need to be copied.
	 */
	Image<Pixel>(const Image<Pixel>& p) : ImageBase(p) {
		pixels = new Pixel[size.pixels];
		std::copy(p.pixels, p.pixels + size.pixels, pixels);
	}

	/**
	 * \brief	Assign an image
	 *
 	 * Note that assignment is only possible if the images have the
	 * same size.
	 */
	Image<Pixel>&	operator=(Image<Pixel>& other) {
		if (!(other.size == size)) {
			throw std::length_error("image frame mismatch");
		}
		std::copy(other.pixels, other.pixels + other.size.pixels, pixels);
	}

	/**
	 * \brief Destroy the image, deallocating the pixel array
	 */
	virtual	~Image() {
		delete pixels;
	}

	/**
	 * \brief A shorthand for the type of the individual pixels
	 */
	typedef	Pixel	pixel_type;

	/**
	 * \brief Read only access to pixel values specified by offset.
	 */
	const Pixel&	operator[](unsigned int offset) const {
		if ((offset < 0) || (offset > size.pixels)) {
			throw std::range_error("offset outside image");
		}
		return pixels[offset];
	}

	/**
	 * \brief Read/write access to pixels specified by offset
	 */
	Pixel&	operator[](unsigned int offset) {
		if ((offset < 0) || (offset > size.pixels)) {
			throw std::range_error("offset outside image");
		}
		return pixels[offset];
	}

	/**
	 * \brief Read only access to pixel values specified by image
	 *        coordinates
	 */
	const Pixel&	pixel(unsigned int x, unsigned int y) const {
		return pixels[pixeloffset(x, y)];
	}

	/**
	 * \brief Read/write access to pixels specified by image coordinates
 	 */
	Pixel&	pixel(unsigned int x, unsigned int y) {
		return pixels[pixeloffset(x, y)];
	}

	// Iterators come either from Rows or from Columns
	class	iterator;
	class	const_iterator;

	/**
	 * \brief The row class of an image constructs iterators for a row
	 */
	class	row : public ImageRow {
		Image<Pixel>	&image;
	public:
		/**
		 * \param _y y coordinate of the row to be constructed
		 */
		row(Image<Pixel> &_image, unsigned int _y)
			: ImageRow(_image.size, _y), image(_image) { }
		iterator	begin();
		const_iterator	begin() const;
		iterator	end();
		const_iterator	end() const;
	};

	/**
	 * \brief The column class of an image constructs iterators for a row
	 */
	class	column : public ImageColumn {
		Image<Pixel>	&image;
	public:
		/**
		 * \param _x x coordinate of the column to be constructed
 		 */
		column(Image<Pixel> &_image, unsigned int _x)
			: ImageColumn(_image.size, _x), image(_image) { }
		iterator	begin();
		const_iterator	begin() const;
		iterator	end();
		const_iterator	end() const;
	};

	/**
	 * \brief Iterator class for an image
	 *
	 * Iterator classes are constructed in such a way as to allow the
	 * use of the STL algorithms to copy image rows, columns, or parts
	 * thereof
	 */
	class iterator : public ImageIteratorBase {
		Image<Pixel>	*image;
	public:
		// pixel iterator constructors
		iterator(Image<Pixel>& _image, unsigned int _first, unsigned int _last,
			unsigned int _offset, unsigned int _stride)
			: ImageIteratorBase(_first, _last, _offset, _stride),
			  image(&_image) { }
		iterator() : image(NULL) { }
		Pixel&	operator*() {
			return image->pixels[pixeloffset()];
		}
		const Pixel&	operator*() const {
			return image->pixels[pixeloffset()];
		}
	};

	/**
	 * \brief Constant iterator for an image
	 */
	class const_iterator : public ImageIteratorBase {
		const Image<Pixel>	&image;
	public:
		const_iterator(const Image<Pixel>& _image,
			unsigned int _first, unsigned int _last,
			unsigned int _offset, unsigned int _stride)
			: ImageIteratorBase(_first, _last, _offset, _stride),
			  image(_image) { }
		const Pixel&	operator*() const {
			return image.pixels[pixeloffset()];
		}
	};

	/**
	 * \brief Compare two images
	 *
	 * Two images are considered equal, if they have the same
	 * dimensions and all pixels have the same values.
	 */
	virtual bool	operator==(const Image<Pixel>& other) const {
		if (!this->ImageBase::operator==(other)) {
			return false;
		}
		return std::equal(pixels, pixels + size.pixels, other.pixels);
	}

	/**
	 * \brief Fill an image with a given value
	 */
	void	fill(const Pixel& value) {
		std::fill(pixels, pixels + size.pixels, value);
	}

	/**
	 * \brief Fill a rectangle of an image with a certain value
	 */
	void	fill(const ImageRectangle& frame, const Pixel& value) {
		for (unsigned int y = 0; y < frame.size.height; y++) {
			ImageRow	r(size, frame.origin.y + y);
			std::fill(pixels + r.firstoffset + frame.origin.x,
				pixels + r.firstoffset + frame.origin.x
					+ size.width, value);
		}
	}

	/**
	 * \brief Access to pixel values
	 * 
 	 */
	template<typename T>
	T	pixelvalue(unsigned int x, unsigned int y) const {
		return (T)pixels[pixeloffset(x, y)];
	}

	/**
	 * \brief Determine number of bits of a pixel
 	 */
	virtual unsigned int	bitsPerPixel() const {
		return astro::image::bitsPerPixel(Pixel());
	}
};

/**
 * This constructor creates a new image which is a subimage of an existing
 * image. It creates a new pixel array, and copies the pixels contained
 * in the frame into the new array.
 *
 * \param src	source image from which to extract the subimage
 * \param frame	specifies the frame around pixel coordinates to include
 *		in the subimage
 */
template<class Pixel>
Image<Pixel>::Image(const Image<Pixel>& src,
		const ImageRectangle& frame) : ImageBase(frame.size) {
	if (!src.size.bounds(frame)) {
		throw std::range_error("subimage frame too large");
	}
	pixels = new Pixel[size.pixels];
	for (unsigned int y = 0; y < frame.size.height; y++) {
		ImageRow	srcrow(src.size, frame.origin.y + y);
		ImageRow	destrow(size, y);
		std::copy(src.pixels + srcrow.firstoffset + frame.origin.x,
			src.pixels + srcrow.firstoffset + frame.origin.x
				+ size.width,
			pixels + destrow.firstoffset);
	}
}

typedef std::tr1::shared_ptr<ImageBase>	ImagePtr;
typedef std::tr1::shared_ptr<Image<unsigned char> >	ByteImagePtr;
typedef std::tr1::shared_ptr<Image<unsigned short> >	ShortImagePtr;
typedef std::tr1::shared_ptr<Image<unsigned int> >	IntImagePtr;
typedef std::tr1::shared_ptr<Image<unsigned long> >	LongImagePtr;
typedef std::tr1::shared_ptr<Image<float> >	FloatImagePtr;
typedef std::tr1::shared_ptr<Image<double> >	DoubleImagePtr;
typedef std::tr1::shared_ptr<Image<RGB<unsigned char> > >	RGBImagePtr;
typedef std::tr1::shared_ptr<Image<YUYV<unsigned char> > >	YUYVImagePtr;

typedef std::vector<ImagePtr>	ImageSequence;

/**
 * \brief Convert a typed image ptr to an untyped image ptr
 *
 * This functions modifies the original pointer so that it no longer
 * points to an image. The actual image is now owned by the new pointer
 */
template<typename P>
ImagePtr	baseimage(std::tr1::shared_ptr<Image<P> >& image) {
	ImagePtr	result(&*image);
	image.reset();
	return result;
}

/**
 * \brief Find out whether an image has a certain type
 *
 * \param image	Image to query the pixel type
 */
template<typename P>
bool	hasType(const ImagePtr& image) {
	return (NULL != dynamic_cast<Image<P> *>(&*image)) ? true : false;
}

/* definitions of the iterator construction methods */
template<class Pixel>
typename Image<Pixel>::iterator	Image<Pixel>::row::begin() {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset, firstoffset, stride);
}

template<class Pixel>
typename Image<Pixel>::iterator	Image<Pixel>::row::end() {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset,
		std::numeric_limits<unsigned int>::max(), stride);
}

template<class Pixel>
typename Image<Pixel>::const_iterator	Image<Pixel>::row::begin() const {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset, firstoffset, stride);
}

template<class Pixel>
typename Image<Pixel>::const_iterator	Image<Pixel>::row::end() const {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset,
		std::numeric_limits<unsigned int>::max(), stride);
}

template<class Pixel>
typename Image<Pixel>::iterator	Image<Pixel>::column::begin() {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset, firstoffset, stride);
}

template<class Pixel>
typename Image<Pixel>::iterator	Image<Pixel>::column::end() {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset,
		std::numeric_limits<unsigned int>::max(), stride);
}

template<class Pixel>
typename Image<Pixel>::const_iterator	Image<Pixel>::column::begin() const {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset, firstoffset, stride);
}

template<class Pixel>
typename Image<Pixel>::const_iterator	Image<Pixel>::column::end() const {
	return typename Image<Pixel>::iterator(image,
		firstoffset, lastoffset,
		std::numeric_limits<unsigned int>::max(), stride);
}

/** 
 * \brief Convert images from one type to another
 *
 * Since we already have functions to convert pixel types to another, we
 * just have to apply the convertPixelArray to the pixel arrays of
 * both images
 */
template<typename destPixel, typename srcPixel>
void	convertImage(Image<destPixel>& dest, const Image<srcPixel>&  src) {
	if (dest.size != src.size) {
		throw std::runtime_error("convertImage: image sizes don't match");
	}
	convertPixelArray(dest.pixels, src.pixels, dest.size.pixels);
}

/**
 * \brief Find out whether an image is a color
 */
bool	isColorImage(const ImagePtr& image);

/**
 * \brief Find out whether an image is a monochrome
 */
bool	isMonochromeImage(const ImagePtr& image);

} // namespace image
} // namespace astro

#endif /* _AstroImage_h */
