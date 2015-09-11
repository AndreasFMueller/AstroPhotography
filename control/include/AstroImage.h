/*
 * AstroImage.h -- abstraction for raw images received from cameras
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroImage_h
#define _AstroImage_h

#include <stdexcept>
#include <memory>
#include <algorithm>
#include <iostream>
#include <AstroPixel.h>
#include <limits>
#include <vector>
#include <map>
#include <list>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <typeinfo>
#include <typeindex>
#include <cmath>

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
	int	_x, _y;
public:
	int	x() const { return _x; }
	int	y() const { return _y; }
	void	x(int x) { _x = x; }
	void	y(int y) { _y = y; }
	ImagePoint(int x = 0, int y = 0) : _x(x), _y(y) { }
	ImagePoint(double x, double y) : _x(floor(x)), _y(floor(y)) { }
	ImagePoint(const std::string& pointspec);
	bool	operator==(const ImagePoint& other) const;
	ImagePoint	operator+(const ImagePoint& other) const;
	ImagePoint	operator-(const ImagePoint& other) const;
	bool	operator<(const ImagePoint& point) const;
	std::string	toString() const;
	operator	std::string() const;
	bool	isZero() const { return (_x == 0) && (_y == 0); }
	float	distance(const ImagePoint& p) const;
};

std::ostream&	operator<<(std::ostream& out, const ImagePoint& point);
std::istream&	operator>>(std::istream& in, ImagePoint& point);

class ImageRectangle;
/**
 * \brief Size of an image or rectangle
 *
 * in the size object we declare all members const, so that users cannot
 * change them destroy the consistency we want to enforce between the
 * number of pixels and width/height.
 */
class ImageSize {
	int	_width, _height;
public:
	// the accessors are defined inline so that a clever compiler can
	// still make them just asf ast as directly accessible members
	int	width() const { return _width; }
	void	setWidth(int width);
	int	height() const { return _height; }
	void	setHeight(int height);
private:
	unsigned int	pixels;
public:
	unsigned int	getPixels() const { return pixels; }
	// constructors
	ImageSize(unsigned int width = 0, unsigned int height = 0);
	ImageSize(const std::string& sizespec);
	virtual ~ImageSize();
	// comparision
	bool	operator==(const ImageSize& other) const;
	bool	operator!=(const ImageSize& other) const;
	// relationships
	bool	bounds(const ImagePoint& point) const;
	bool	bounds(const ImageRectangle& rectangle) const;
	bool	contains(const ImagePoint& point) const;
	bool	contains(int x, int y) const;
	// characteristic function
	int	chi(unsigned int x, unsigned int y) const;
	// offset into pixel array
	unsigned int	offset(unsigned int x, unsigned int y) const;
	unsigned int	offset(const ImagePoint& point) const;
	// text representation
	virtual std::string	toString() const;
	// corners
	ImagePoint	upperright() const;
	ImagePoint	upperleft() const;
	ImagePoint	lowerleft() const;
	ImagePoint	lowerright() const;
	ImagePoint	center() const;
	// scaling
	ImageSize	operator*(const double l) const;
	// reduction 
	ImagePoint	operator()(const int x, const int y) const;
	ImagePoint	operator()(const ImagePoint& p) const;
	// find out whether the rectangle is uninitialized
	bool	isEmpty() const { return (_width == 0) && (_height == 0); }
};

std::ostream&	operator<<(std::ostream& out, const ImageSize& size);
std::istream&	operator>>(std::istream& in, ImageSize& point);

/**
 * \brief Rectangle
 *
 * The ImageRectangle abstraction is used to specify rectangles within an
 * image. An ImageRectangle is specified by a ImagePoint, the origin, which
 * is the lower left corner of the rectangle, and a an ImageSize, which
 * specifies width and height of the rectangle,
 */
class ImageRectangle {
	ImagePoint	_origin;
	ImageSize	_size;
public:
	// accessors
	const ImagePoint&	origin() const { return _origin; }
	const ImageSize&	size() const { return _size; }
	void	setOrigin(const ImagePoint& origin) { _origin = origin; }
	void	setSize(const ImageSize& size) { _size = size; }
	// constructors
	ImageRectangle(unsigned int w = 0, unsigned int h = 0) : _size(w, h) { }
	ImageRectangle(const ImageSize& size) : _size(size) { }
	ImageRectangle(const ImagePoint& origin, const ImageSize& size)
		: _origin(origin), _size(size) { }
	ImageRectangle(const ImageRectangle& rectangle,
		const ImagePoint& translatedby);
	ImageRectangle(const ImageRectangle& rectangle,
		const ImageRectangle& subrectangle);
	ImageRectangle(const std::string& rectanglespec);
	// oeprators
	bool	contains(const ImagePoint& point) const;
	bool	contains(const ImageRectangle& rectangle) const;
	bool	fits(const ImageSize& size) const;
	bool	operator==(const ImageRectangle& other) const;
	const ImagePoint&	lowerLeftCorner() const;
	ImagePoint	lowerRightCorner() const;
	ImagePoint	upperRightCorner() const;
	ImagePoint	upperLeftCorner() const;
	// text represenation
	std::string	toString() const;
	operator	std::string() const;
	// corners
	ImagePoint	upperright() const;
	ImagePoint	upperleft() const;
	ImagePoint	lowerleft() const;
	ImagePoint	lowerright() const;
	// center
	ImagePoint	center() const;
	// find out whether the rectangle is uninitialized
	bool	isEmpty() const { return _size.isEmpty(); }
	// subimage
	ImagePoint	subimage(const ImagePoint& point) const;
	ImagePoint	subimage(int x, int y) const;
};

std::ostream&	operator<<(std::ostream& out, const ImageRectangle& rectangle);
std::istream&	operator>>(std::istream& in, ImageRectangle& rectangle);

/**
 * \brief Object representing a date in a FITS header
 *
 * FITS files contain date / time information in a special format, this
 * class converts it to a Unix struct timeval and is able to format
 * in different forms.
 */
class FITSdate {
	struct timeval	when;
public:
	FITSdate(const std::string& date);
	FITSdate(time_t t);
	FITSdate(const struct timeval& tv);
	FITSdate();
	struct timeval	time() const { return when; }
	std::string	showShort() const;
	std::string	showLong() const;
	std::string	showVeryLong() const;
	bool	operator==(const FITSdate& other) const;
	bool	operator<(const FITSdate& other) const;
	operator	time_t() const { return when.tv_sec; }
	operator	struct timeval() const { return when; }
	operator	std::string() const { return showVeryLong(); }
};

/**
 * \brief Image metadata is stored in a multi map
 */
class Metavalue {
	std::string	keyword;
	std::type_index	datatype;
	std::string	value;
	std::string	comment;
	void	standardize();
public:
	const std::string&	getKeyword() const { return keyword; }
	const std::string&	getValue() const { return value; }
	const std::string&	getComment() const { return comment; }
	std::type_index	getType() const { return datatype; }
	Metavalue(const std::string& _keyword, const std::string& _value,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const bool b,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const char c,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const unsigned char uc,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const short s,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const unsigned short us,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const int i,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const unsigned int ui,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const long l,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const unsigned long ul,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const float f,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const double f,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, const FITSdate& d,
		const std::string& _comment);
	Metavalue(const std::string& _keyword, std::type_index _datatype,
		const std::string& _value, const std::string& _comment);
	operator	bool() const;
	operator	char() const;
	operator	unsigned char() const;
	operator	short() const;
	operator	unsigned short() const;
	operator	int() const;
	operator	unsigned int() const;
	operator	long() const;
	operator	unsigned long() const;
	operator	float() const;
	operator	double() const;
	operator	std::string() const;
	operator	FITSdate() const;
	std::string	toString() const;
	bool	operator==(const Metavalue& other) const;
};

/**
 * \brief A class that is aware of valid FITS keys
 */
class ImageMetadata : public std::list<std::pair<std::string, Metavalue> > {
public:
	bool	hasMetadata(const std::string& keyword) const;
	const Metavalue&	getMetadata(const std::string& keyword) const;
	void	setMetadata(const Metavalue& mv);
	ImageMetadata::const_iterator	find(const std::string& keyword) const;
	ImageMetadata::iterator	find(const std::string& keyword);
	void	remove(const std::string& keyword);
	void	dump() const;
};

/**
 * \brief MosaicType
 *
 * Bayer RGB mosaic type
 */
class MosaicType {
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
private:
	mosaic_type	mosaic;
public:
	MosaicType(mosaic_type _mosaic = NONE) : mosaic(_mosaic) { }
	mosaic_type	getMosaicType() const { return mosaic; }
	void	setMosaicType(mosaic_type mosaic);
	void	setMosaicType(const std::string& mosaic_name);
	bool	isMosaic() const;
	// methods used for demosaicing: x/y coordinates of colored
	// pixels
	ImagePoint	red() const;
	ImagePoint	blue() const;
	ImagePoint	greenb() const;
	ImagePoint	greenr() const;
	// methods related to the mosaic stuff
	bool	isR(unsigned int x, unsigned int y) const;
	bool	isG(unsigned int x, unsigned int y) const;
	bool	isB(unsigned int x, unsigned int y) const;
	bool	isGr(unsigned int x, unsigned int y) const;
	bool	isGb(unsigned int x, unsigned int y) const;
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
class ImageBase : public Typename {
	/**
	 * \brief Metadata for the image
	 *
	 * Some of the Metadata is not accessible 
 	 */
	ImageMetadata	metadata;
public:
	// access to metadata
	bool	hasMetadata(const std::string& name) const;
	Metavalue	getMetadata(const std::string& name) const;
	void	removeMetadata(const std::string& name);
	void	setMetadata(const Metavalue& mv);
	int	nMetadata() const { return metadata.size(); }
	ImageMetadata::const_iterator	begin() const;
	ImageMetadata::const_iterator	end() const;
	void	dump_metadata() const;
protected:
	MosaicType	mosaic;
public:
	// accessors for metadata
	MosaicType	getMosaicType() const { return mosaic; }
	void	setMosaicType(MosaicType::mosaic_type mosaic);
	void	setMosaicType(const std::string& mosaic_name);

	// the size is publicly accessible, but users should not change it
protected:
	ImageRectangle	frame;
public:
	const ImageRectangle&	getFrame() const { return frame; }
	void	setOrigin(const ImagePoint& origin) { frame.setOrigin(origin); }
	const ImageSize&	size() const { return frame.size(); }
	const ImagePoint&	origin() const { return frame.origin(); }
	ImagePoint	center() const { return size().center(); }

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

	virtual unsigned int bitsPerPixel() const { return 0; }
	virtual unsigned int bytesPerPixel() const { return 0; }
	virtual unsigned int	planes() const { return 0; }
	unsigned int bytesPerPlane() const;
	unsigned int bitsPerPlane() const;

	// pixel range stuff
	virtual double	minimum() const { return 0; }
	virtual double	maximum() const { return 255; }

	// text representation (for debugging)
	friend std::ostream&	operator<<(std::ostream& out,
		const ImageBase& image);

	// type index of the pixel type
	virtual std::type_index	pixel_type() const;
};

std::ostream&	operator<<(std::ostream& out, const ImageBase& image);

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
		: ImageLine(size.width() * _y, size.width() * (_y + 1) - 1, 1),
		  y(_y) { }
};

/**
 * \brief Base class for column iterators
 */
class	ImageColumn : public ImageLine {
public:
	const unsigned int	x;
	ImageColumn(const ImageSize& size, unsigned int _x)
		: ImageLine(_x, _x + size.getPixels() - size.width(),
			size.width()), x(_x) { }
};

/**
 * \brief Read-only Access to the pixels of an image
 *
 * The Image class gives some basic access to the pixels of an image.
 * more sophisticated access, like selecting planes, merging planes,
 * converting pixel type, taking subimages etc. is handled through 
 * adapter classes. This is the base class for these adapter classes,
 * it defines the pixel accessors. 
 */
template<typename Pixel>
class ConstImageAdapter {
protected:
	ImageSize	adaptersize;
public:
	/**
	 * \brief A shorthand for the type of the individual pixels
	 */
	typedef	Pixel	pixel_type;

	ConstImageAdapter(const ImageSize& _size) : adaptersize(_size) { }
	virtual ~ConstImageAdapter() { }

	ImageSize	getSize() const { return adaptersize; }
	virtual Pixel	pixel(int x, int y) const = 0;
	Pixel	pixel(ImagePoint p) const {
		return pixel(p.x(), p.y());
	}
};

/**
 * \brief Read-write Access to the pixels of an image
 *
 */
template<typename Pixel>
class ImageAdapter : public ConstImageAdapter<Pixel> {
public:
	ImageAdapter(const ImageSize& size) : ConstImageAdapter<Pixel>(size) {}
	virtual Pixel&	writablepixel(int x, int y) = 0;
};


/**
 * \brief Image class
 *
 * The Image template class implements images with different pixel types
 * as specified by the template argument. Images have an immutable size
 */
template<typename Pixel>
class Image : public ImageBase, public ImageAdapter<Pixel> {
public:
	/**
	 * \brief Access to the image size
	 */
	virtual	ImageSize	getSize() const {
		return frame.size();
	}

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
		: ImageBase(_w, _h), ImageAdapter<Pixel>(ImageSize(_w, _h)) {
		if (p) {
			pixels = p;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "taking ownership of "
				"%d pixels for image %s at %p",
				frame.size().getPixels(),
				frame.size().toString().c_str(), pixels);
		} else {
			pixels = new Pixel[frame.size().getPixels()];
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"alloc %d pixels for image %s at %p",
				frame.size().getPixels(),
				frame.size().toString().c_str(), pixels);
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
	Image<Pixel>(const ImageSize& size, Pixel *p = NULL)
		: ImageBase(size), ImageAdapter<Pixel>(size) {
		if (p) {
			pixels = p;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "taking ownership of "
				"%d pixels for image %s at %p",
				frame.size().getPixels(),
				frame.size().toString().c_str(), pixels);
		} else {
			pixels = new Pixel[size.getPixels()];
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"alloc %d pixels for image %s at %p",
				size.getPixels(),
				size.toString().c_str(), pixels);
		}
	}

	/**
	 * \brief Create an image from an adapter
	 *
	 * Usually, adapters are only "virtual" images, the pixels are
	 * computed only when needed. In some cases, like when an image is
	 * to be stored in a file, a concrete image has to be instantiated
	 * from the adapter
 	 */ 
	Image<Pixel>(const ConstImageAdapter<Pixel>& adapter)
		: ImageBase(adapter.getSize()),
		  ImageAdapter<Pixel>(adapter.getSize()) {
		pixels = new Pixel[frame.size().getPixels()];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %d pixels at %p",
			frame.size().toString().c_str(),
			frame.size().getPixels(), pixels);
		for (int x = 0; x < frame.size().width(); x++) {
			for (int y = 0; y < frame.size().height(); y++) {
				pixel(x, y) = adapter.pixel(x, y);
			}
		}
	}

	/**
	 * \brief	Copy an image from a different pixel type
	 *
	 * \param other	image to copy
 	 */
	template<typename srcPixel>
	Image<Pixel>(const Image<srcPixel>& other)
		: ImageBase(other.size()),
		  ImageAdapter<Pixel>(other.size()) {
		pixels = new Pixel[frame.size().getPixels()];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %d pixels at %p",
			frame.size().toString().c_str(),
			frame.size().getPixels(), pixels);
		convertPixelArray(pixels, other.pixels,
			frame.size().getPixels());
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
	 *
	 * \param p	image to copy
	 */
	Image<Pixel>(const Image<Pixel>& p) : ImageBase(p),
		ImageAdapter<Pixel>(p.frame.size()) {
		pixels = new Pixel[frame.size().getPixels()];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %d pixels at %p",
			frame.size().toString().c_str(),
			frame.size().getPixels(), pixels);
		std::copy(p.pixels, p.pixels + frame.size().getPixels(), pixels);
	}

	/**
	 * \brief	Assign an image
	 *
 	 * Note that assignment is only possible if the images have the
	 * same size.
	 */
	Image<Pixel>&	operator=(Image<Pixel>& other) {
		if (!(other.frame.size() == frame.size())) {
			throw std::length_error("image frame mismatch");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy pixels %p -> %p",
			other.pixels, pixels);
		std::copy(other.pixels, other.pixels + other.frame.size().getPixels(), pixels);
		return *this;
	}

	/**
	 * \brief Destroy the image, deallocating the pixel array
	 */
	virtual	~Image() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "delete pixels at %p", pixels);
		delete[] pixels;
	}

	/**
	 * \brief Read only access to pixel values specified by offset.
	 */
	const Pixel&	operator[](unsigned int offset) const {
		if (offset > frame.size().getPixels()) {
			throw std::range_error("offset outside image");
		}
		return pixels[offset];
	}

	/**
	 * \brief Read/write access to pixels specified by offset
	 */
	Pixel&	operator[](unsigned int offset) {
		if (offset > frame.size().getPixels()) {
			throw std::range_error("offset outside image");
		}
		return pixels[offset];
	}

	/**
	 * \brief Read only access to pixel values specified by image
	 *        coordinates
	 */
	Pixel	pixel(int x, int y) const {
		return pixels[pixeloffset(x, y)];
	}

	Pixel	pixel(const ImagePoint& p) const {
		return pixel(p.x(), p.y());
	}

	/**
	 * \brief Read/write access to pixels specified by image coordinates
 	 */
	Pixel&	pixel(int x, int y) {
		return pixels[pixeloffset(x, y)];
	}

	Pixel&	pixel(const ImagePoint& p) {
		return pixel(p.x(), p.y());
	}

	Pixel&	writablepixel(int x, int y) {
		return pixels[pixeloffset(x, y)];
	}

	Pixel&	writablepixel(const ImagePoint& p) {
		return writablepixel(p.x(), p.y());
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
		row(Image<Pixel> &_image, int _y)
			: ImageRow(_image.frame.size(), _y), image(_image) { }
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
		column(Image<Pixel> &_image, int _x)
			: ImageColumn(_image.size(), _x), image(_image) { }
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
		return std::equal(pixels, pixels + frame.size().getPixels(), other.pixels);
	}

	/**
	 * \brief Fill an image with a given value
	 */
	void	fill(const Pixel& value) {
		std::fill(pixels, pixels + frame.size().getPixels(), value);
	}

	/**
	 * \brief Fill a rectangle of an image with a certain value
	 */
	void	fill(const ImageRectangle& subframe, const Pixel& value) {
		for (int y = 0; y < subframe.size().height(); y++) {
			ImageRow	r(frame.size(), subframe.origin().y() + y);
			std::fill(pixels + r.firstoffset + subframe.origin().x(),
				pixels + r.firstoffset + subframe.origin().x()
					+ frame.size().width(), value);
		}
	}

	/**
	 * \brief Clear an image
 	 */
	void	clear() {
		fill(0);
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
	 *
	 * For floating point values, this is the mantissa size. So this
	 * value gives information about the resolution.
 	 */
	virtual unsigned int	bitsPerPixel() const {
		return astro::image::bitsPerPixel(Pixel());
	}

	/**
 	 * \brief Determine the number of bytes per pixel
	 *
	 * This value gives information about the storage requirements of
	 * a pixel. For doubles, this returns 8, while the bitsPerPixel
	 * method returns only 53 for the 53 significant digits of the
	 * mantissa.
 	 */
	virtual unsigned int	bytesPerPixel() const {
		return sizeof(Pixel);
	}

	/**
	 * \brief Determine the number of planes
	 */
	virtual unsigned int	planes() const {
		return astro::image::planes(Pixel());
	}

	/**
	 * \brief get maximum pixel value
 	 */
	virtual double	maximum() const {
		return pixel_maximum<Pixel>();
	}

	virtual std::type_index	pixel_type() const {
		return std::type_index(typeid(Pixel));
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
		const ImageRectangle& subframe)
	: ImageBase(subframe.size()), ImageAdapter<Pixel>(subframe.size()) {
	if (!src.frame.size().bounds(subframe)) {
		throw std::range_error("subimage frame too large");
	}
	pixels = new Pixel[subframe.size().getPixels()];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "alloc %d bytes for subframe %s at %p",
		subframe.size().getPixels(), subframe.size().toString().c_str(),
		pixels);
	for (int y = 0; y < subframe.size().height(); y++) {
		ImageRow	srcrow(src.frame.size(), subframe.origin().y() + y);
		ImageRow	destrow(frame.size(), y);
		std::copy(src.pixels + srcrow.firstoffset + subframe.origin().x(),
			src.pixels + srcrow.firstoffset + subframe.origin().x()
				+ frame.size().width(),
			pixels + destrow.firstoffset);
	}
}

typedef std::shared_ptr<ImageBase>	ImagePtr;
typedef std::shared_ptr<Image<unsigned char> >	ByteImagePtr;
typedef std::shared_ptr<Image<unsigned short> >	ShortImagePtr;
typedef std::shared_ptr<Image<unsigned int> >	IntImagePtr;
typedef std::shared_ptr<Image<unsigned long> >	LongImagePtr;
typedef std::shared_ptr<Image<float> >	FloatImagePtr;
typedef std::shared_ptr<Image<double> >	DoubleImagePtr;
typedef std::shared_ptr<Image<RGB<unsigned char> > >	RGBImagePtr;
typedef std::shared_ptr<Image<YUYV<unsigned char> > >	YUYVImagePtr;

typedef std::vector<ImagePtr>	ImageSequence;

/**
 * \brief Convert a typed image ptr to an untyped image ptr
 *
 * This functions modifies the original pointer so that it no longer
 * points to an image. The actual image is now owned by the new pointer
 */
template<typename P>
ImagePtr	baseimage(std::shared_ptr<Image<P> >& image) {
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
	if (dest.size() != src.size()) {
		throw std::runtime_error("convertImage: image sizes don't match");
	}
	convertPixelArray(dest.pixels, src.pixels, dest.size().getPixels());
}

/**
 * \brief Copy the contents of an adapter into an image
 */
template<typename destPixel, typename srcPixel>
void	copy(Image<destPixel>& dest, const ConstImageAdapter<srcPixel>& src) {
	if (dest.getSize() != src.getSize()) {
		throw std::range_error("cannot copy images of different size");
	}
	int	w = src.getSize().width();
	int	h = src.getSize().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			dest.pixel(x, y) = src.pixel(x, y);
		}
	}
}

/**
 * \brief Find out whether an image is a color
 */
bool	isColorImage(const ImagePtr& image);

/**
 * \brief Find out whether an image is a monochrome
 */
bool	isMonochromeImage(const ImagePtr& image);

/**
 * \brief Abstraction for subgrids of an image
 */
class Subgrid {
public:
	ImagePoint	origin;
	ImageSize	stepsize;
	Subgrid();
	Subgrid(const ImagePoint& origin, const ImageSize& stepsize);
	Subgrid(const Subgrid& other);
	unsigned int	x(unsigned int _x) const;
	unsigned int	y(unsigned int _y) const;
	unsigned int	volume() const;
	std::string	toString() const;
};

/**
 * \brief FWHM information
 */
class FWHMInfo {
public:
	ImagePoint	maxpoint;
	double		maxvalue;
	ImagePtr	mask;
	ImagePoint	center;
	double		radius;
};

/**
 * \brief Information about focus quality
 */
class FocusInfo {
public:
	double		value;
	ImagePtr	edges;
};

} // namespace image
} // namespace astro

#endif /* _AstroImage_h */
