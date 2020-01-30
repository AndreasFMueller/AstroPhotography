/*
 * AstroImage.h -- abstraction for raw images received from cameras
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
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
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroStatistics.h>

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
	bool	operator!=(const ImagePoint& other) const;
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
	int	smallerSide() const { return std::min(_width, _height); }
	int	largerSide() const { return std::max(_width, _height); }
private:
	unsigned int	pixels;
public:
	unsigned int	getPixels() const { return pixels; }
	// constructors
	ImageSize(unsigned int width, unsigned int height);
	ImageSize(const std::string& sizespec);
	ImageSize(unsigned int width_and_height = 0);
	ImageSize(const ImageSize& other);
	ImageSize&	operator=(const ImageSize& other);
	virtual ~ImageSize();
	// comparision
	bool	operator==(const ImageSize& other) const;
	bool	operator!=(const ImageSize& other) const;
	// relationships
	bool	bounds(const ImagePoint& point) const;
	bool	bounds(const ImageRectangle& rectangle) const;
	bool	contains(const ImagePoint& point) const;
	bool	contains(int x, int y) const;
	ImageRectangle	containing(const ImageRectangle& rectangle) const;
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
	ImagePoint	centerWithinRectangle(const ImageSize& frame) const;
	// scaling
	ImageSize	operator*(const double l) const;
	ImageSize	operator/(const double l) const;
	// reduction 
	ImagePoint	operator()(const int x, const int y) const;
	ImagePoint	operator()(const ImagePoint& p) const;
	// flip operations for points inside an image rectangle
	ImagePoint	flip(const ImagePoint& p) const;
	ImagePoint	horizontalFlip(const ImagePoint& p) const;
	ImagePoint	verticalFlip(const ImagePoint& p) const;
	// find out whether the rectangle is uninitialized
	bool	isEmpty() const { return (_width == 0) && (_height == 0); }
	// border distance
	int	borderDistance(const ImagePoint& point) const;
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
	ImageRectangle(const ImageSize& size, int boundarydistance = 0);
	ImageRectangle(const ImagePoint& origin, const ImageSize& size)
		: _origin(origin), _size(size) { }
	ImageRectangle(const ImageRectangle& rectangle,
		const ImagePoint& translatedby);
	ImageRectangle(const ImageRectangle& rectangle,
		const ImageRectangle& subrectangle);
	ImageRectangle(const std::string& rectanglespec);
	ImageRectangle&	operator=(const ImageRectangle& other);
	// operators
	bool	contains(int x, int y) const;
	bool	contains(const ImagePoint& point) const;
	bool	contains(const ImageRectangle& rectangle) const;
	bool	fits(const ImageSize& size) const;
	bool	operator==(const ImageRectangle& other) const;
	bool	operator!=(const ImageRectangle& other) const;
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
	ImagePoint	centerWithinFrame(const ImageSize& frame) const;
	// find out whether the rectangle is uninitialized
	bool	isEmpty() const { return _size.isEmpty(); }
	// subimage
	ImagePoint	subimage(const ImagePoint& point) const;
	ImagePoint	subimage(int x, int y) const;
	// border distance
	int	borderDistance(const ImagePoint& point) const;
	// limits of the ranges
	int	xmin() const;
	int	ymin() const;
	int	xmax() const;
	int	ymax() const;
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
	static mosaic_type	string2type(const std::string& mosaic_name);
	static std::string	type2string(mosaic_type t);
public:
	// geometric operations on the mosaic type
	static mosaic_type	shift(mosaic_type t, const ImagePoint& offset);
	static mosaic_type	vflip(mosaic_type t);
	static mosaic_type	hflip(mosaic_type t);
	static mosaic_type	rotate(mosaic_type t);
	// constructors
	MosaicType(mosaic_type _mosaic = NONE,
		ImagePoint offset = ImagePoint());
	MosaicType(const std::string& mosaicstring,
		ImagePoint offset = ImagePoint());
	// handling the mosaic type
	mosaic_type	getMosaicType() const { return mosaic; }
	void	setMosaicType(mosaic_type mosaic,
			ImagePoint offset = ImagePoint());
	void	setMosaicType(const std::string& mosaic_name,
			ImagePoint offset = ImagePoint());
	bool	isMosaic() const;
	// typecast
	operator	std::string() const { return type2string(mosaic); }
	operator	bool() const { return mosaic != NONE; }
	// methods used for demosaicing: x/y coordinates of colored pixels
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
	// methods giving the mosaic type of a subrectangle
	MosaicType	shifted(const ImagePoint& offset) const;
	MosaicType	shifted(const ImageRectangle& rectangle) const;
	MosaicType	operator()(const ImagePoint& offset) const;
	MosaicType	operator()(const ImageRectangle& rectangle) const;
	// methods used for vertical flipping of the mosaic
	MosaicType	vflip() const;
	MosaicType	hflip() const;
	MosaicType	rotate() const;
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
	ImageMetadata	_metadata;
public:
	// access to metadata
	const ImageMetadata	metadata() const { return _metadata; }
	void	metadata(const ImageMetadata& im) { _metadata = im; }
	bool	hasMetadata(const std::string& name) const;
	Metavalue	getMetadata(const std::string& name) const;
	void	removeMetadata(const std::string& name);
	void	setMetadata(const Metavalue& mv);
	int	nMetadata() const { return _metadata.size(); }
	ImageMetadata::const_iterator	begin() const;
	ImageMetadata::const_iterator	end() const;
	void	dump_metadata() const;
protected:
	MosaicType	mosaic;
public:
	// accessors for mosaic type
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
	virtual unsigned int bytesPerPlane() const { return 0; }
	virtual unsigned int bitsPerPlane() const { return 0; }

	// pixel range stuff
	virtual double	minimum() const { return 0; }
	virtual double	maximum() const { return 255; }

	// text representation (for debugging)
	friend std::ostream&	operator<<(std::ostream& out,
		const ImageBase& image);

	// type index of the pixel type
	virtual std::type_index	pixel_type() const;

	// info string for the image
	virtual std::string	info() const;

	// add color space information
	void	addColorspace(const monochrome_color_tag&);
	void	addColorspace(const multiplane_color_tag&);
	void	addColorspace(const yuv_color_tag&);
	void	addColorspace(const yuyv_color_tag&);
	void	addColorspace(const rgb_color_tag&);
	void	addColorspace(const xyz_color_tag&);
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
	unsigned int	pixeloffset() const;
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
 * \brief Common base class for all adapters
 *
 * This class simplifies resource management, because now all adapters
 * can be reference by a shared pointer to this type, and will correctly
 * be deallocated when the last reference goes out of scope
 */
class BasicAdapter {
protected:
	ImageSize	adaptersize;
public:
	BasicAdapter(const ImageSize& size);
	virtual ~BasicAdapter();
	const ImageSize&	getSize() const;
};
typedef std::shared_ptr<BasicAdapter>	BasicAdapterPtr;

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
class ConstImageAdapter : public BasicAdapter {
public:
	/**
	 * \brief A shorthand for the type of the individual pixels
	 */
	typedef	Pixel	pixel_type;

	ConstImageAdapter(const ImageSize& _size) : BasicAdapter(_size) { }
	virtual ~ConstImageAdapter() { }

	virtual Pixel	pixel(int x, int y) const = 0;
	Pixel	pixel(const ImagePoint& p) const {
		return pixel(p.x(), p.y());
	}

	/**
	 * \brief Give some information about the image (including pixel type)
	 */
	virtual std::string	info() const {
		return demangle(typeid(*this).name());
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
	Pixel&	writablepixel(const ImagePoint& p) {
		return writablepixel(p.x(), p.y());
	}
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
		addColorspace(typename color_traits<Pixel>::color_category());
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
			statistics::Memory::image_allocate(
				frame.size().getPixels(), sizeof(Pixel));
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
		addColorspace(typename color_traits<Pixel>::color_category());
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
			statistics::Memory::image_allocate(
				size.getPixels(), sizeof(Pixel));
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
	template<typename srcPixel>
	Image<Pixel>(const ConstImageAdapter<srcPixel>& adapter)
		: ImageBase(adapter.getSize()),
		  ImageAdapter<Pixel>(adapter.getSize()) {
		addColorspace(typename color_traits<Pixel>::color_category());
		long	number_of_pixels = frame.size().getPixels();
		pixels = new Pixel[number_of_pixels];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %ld pixels at %p",
			frame.size().toString().c_str(), number_of_pixels,
			pixels);
		statistics::Memory::image_allocate(number_of_pixels,
			sizeof(Pixel));
#		pragma omp parallel for
		for (int x = 0; x < frame.size().width(); x++) {
			for (int y = 0; y < frame.size().height(); y++) {
				pixel(x, y) = adapter.pixel(x, y);
			}
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
	template<typename srcPixel>
	Image<Pixel>(const ConstImageAdapter<srcPixel>& adapter,
		double scalefactor)
		: ImageBase(adapter.getSize()),
		  ImageAdapter<Pixel>(adapter.getSize()) {
		addColorspace(typename color_traits<Pixel>::color_category());
		long	number_of_pixels = frame.size().getPixels();
		pixels = new Pixel[number_of_pixels];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %d pixels at %p",
			frame.size().toString().c_str(),
			frame.size().getPixels(), pixels);
		statistics::Memory::image_allocate(number_of_pixels,
			sizeof(Pixel));
#		pragma omp parallel for
		for (int x = 0; x < frame.size().width(); x++) {
			for (int y = 0; y < frame.size().height(); y++) {
//debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %4d, y = %4d, scalefactor = %f", x, y, scalefactor);
				pixel(x, y) = adapter.pixel(x, y) * scalefactor;
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
		addColorspace(typename color_traits<Pixel>::color_category());
		pixels = new Pixel[frame.size().getPixels()];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %d pixels at %p",
			frame.size().toString().c_str(),
			frame.size().getPixels(), pixels);
		statistics::Memory::image_allocate(frame.size().getPixels(),
			sizeof(Pixel));
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
		addColorspace(typename color_traits<Pixel>::color_category());
		pixels = new Pixel[frame.size().getPixels()];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %d pixels at %p",
			frame.size().toString().c_str(),
			frame.size().getPixels(), pixels);
		statistics::Memory::image_allocate(frame.size().getPixels(),
			sizeof(Pixel));
		std::copy(p.pixels, p.pixels + frame.size().getPixels(), pixels);
	}

	/**
	 * \brief Copy and rescale an image
	 *
	 * This constructor copies an image and rescales it on the fly
	 *
	 * \param p		image to copy
	 * \param scalefactor	factor by which to scale the image
	 */
	template<typename srcPixel>
	Image<Pixel>(const Image<srcPixel>& p, double scalefactor)
		: ImageBase(p), ImageAdapter<Pixel>(p.getFrame().size()) {
		addColorspace(typename color_traits<Pixel>::color_category());
		long	number_of_pixels = frame.size().getPixels();
		pixels = new Pixel[number_of_pixels];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %s alloc %d pixels at %p",
			frame.size().toString().c_str(),
			frame.size().getPixels(), pixels);
		statistics::Memory::image_allocate(number_of_pixels,
			sizeof(Pixel));
		for (long i = 0; i < number_of_pixels; i++) {
			pixels[i] = p.pixels[i] * scalefactor;
		}
	}

	/**
	 * \brief	Assign an image
	 *
 	 * Note that assignment is only possible if the images have the
	 * same size.
	 */
	Image<Pixel>&	operator=(Image<Pixel>& other) {
		if (!(other.frame.size() == frame.size())) {
			std::string	msg = astro::stringprintf(
				"mismatch: copy %s to %s",
				other.frame.size().toString().c_str(),
				frame.size().toString().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::length_error(msg);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "copy pixels %p -> %p",
			other.pixels, pixels);
		std::copy(other.pixels, other.pixels + other.frame.size().getPixels(), pixels);
		return *this;
	}

	/**
 	 * \brief Copy an image from an image adapter
	 *
	 * This is less efficient than the image copy operation above
	 */
	Image<Pixel>&	operator=(ConstImageAdapter<Pixel>& other) {
		if (!(other.getSize() == frame.size())) {
			std::string	msg = astro::stringprintf(
				"mismatch: copy %s to %s",
				other.getSize().toString().c_str(),
				frame.size().toString().c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::length_error(msg);
		}
#		pragma omp parallel for
		for (int x = 0; x < other.getSize().width(); x++) {
			for (int y = 0; y < other.getSize().height(); y++) {
				pixel(x, y) = other.pixel(x, y);
			}
		}
		return *this;
	}

	/**
	 * \brief Destroy the image, deallocating the pixel array
	 */
	virtual	~Image() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "delete pixels at %p", pixels);
		delete[] pixels;
		statistics::Memory::image_deallocate(frame.size().getPixels(),
			sizeof(Pixel));
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

public:
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

private:
	/**
	 * \brief Get pointer into the pixel array
	 *
	 * \param x	x-coordinate of pixel
	 * \param y	y-coordinate of pixel
	 */
	Pixel	*pixel_pointer(int x, int y) {
		return &pixels[y * frame.size().width() + x];
	}

	/**
	 * \brief Get pointer into the pixel array
	 *
	 * \param p	Point in the image
	 */
	Pixel	*pixel_pointer(ImagePoint p) {
		return pixel_pointer(p.x(), p.y());
	}

public:
	/**
	 * \brief Flip an image vertically
	 *
	 * Flip an image vertically
	 */
	void	flip() {
		int	h = frame.size().height();
		int	w = frame.size().width();
		int	l = h / 2;
		for (int y = 0; y < l; y++) {
			Pixel	*first1 = pixel_pointer(0, y);
			Pixel	*last1 = pixel_pointer(w, y);
			Pixel	*first2 = pixel_pointer(0, h - 1 - y);
			std::swap_ranges(first1, last1, first2);
		}
		if (0 == (h % 2)) {
			mosaic = mosaic.vflip();
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
	 *
	 * For floating point values, this is the mantissa size. So this
	 * value gives information about the resolution.
 	 */
	virtual unsigned int	bitsPerPixel() const {
		return astro::image::bitsPerPixel(Pixel());
	}

	/**
	 * \brief bits per Value
	 */
	virtual unsigned int	bitsPerPlane() const {
		return astro::image::bitsPerValue(Pixel());
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
		return astro::image::bytesPerPixel(Pixel());
	}

	/**
 	 * \brief bytes per Value
	 */
	virtual unsigned int	bytesPerPlane() const {
		return astro::image::bytesPerValue(Pixel());
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

	/**
	 * \brief Add another image to the present image
	 */
	template<class otherPixel>
	void	add(const Image<otherPixel>& otherImage) {
		// check for matching image size
		if (size() != otherImage.size()) {
			throw std::runtime_error("image size mismatch");
		}
		for (int x = 0; x < size().width(); x++) {
			for (int y = 0; y < size().height(); y++) {
				Pixel	p = pixel(x, y);
				pixel(x, y) = p + otherImage.pixel(x, y);
			}
		}
	}

	/**
	 * \brief Make the image positive
	 */
	void	absolute() {
		for (int x = 0; x < size().width(); x++) {
			for (int y = 0; y < size().height(); y++) {
				Pixel	p = pixel(x, y);
				if (p < 0) {
					pixel(x, y) = -p;
				}
			}
		}
	}

	/**
	 * \brief Make the positive part of an image 
	 */
	void	positive() {
		for (int x = 0; x < size().width(); x++) {
			for (int y = 0; y < size().height(); y++) {
				Pixel	p = pixel(x, y);
				if (p < 0) {
					pixel(x, y) = 0;
				}
			}
		}
	}

	/**
	 * \brief Make the negative part of an image
	 */
	void	negative() {
		for (int x = 0; x < size().width(); x++) {
			for (int y = 0; y < size().height(); y++) {
				Pixel	p = pixel(x, y);
				if (p > 0) {
					pixel(x, y) = 0;
				}
			}
		}
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
	statistics::Memory::image_allocate(subframe.size().getPixels(),
			sizeof(Pixel));
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
bool	hasType(const ImagePtr image) {
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
bool	isColorImage(const ImagePtr image);

/**
 * \brief Find out whether an image is a monochrome
 */
bool	isMonochromeImage(const ImagePtr image);

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

/**
 * \brief Binning mode specification
 *
 * many astrophotography cameras support binning, i.e. combining adjacent pixels
 * to form larger pixels. The coordinate values in a binning object
 * can also be set to -1, which means that any positive number would
 * be valid. This simplifies specifying the acceptable binning modes as a list
 * Binning objects.
 */
class	Binning {
	int	_x;
	int	_y;
public:
	int	x() const { return _x; }
	void	x(int v) { _x = v; }
	int	y() const { return _y; }
	void	y(int v) { _y = v; }
public:
	Binning(int x = 1, int y = 1);
	Binning(const Binning& other) : _x(other._x), _y(other._y) { }
	Binning(const std::string& binning);
	Binning(const ImageBase* imagebase);
	Binning(const ImagePtr imagebase);
	virtual	~Binning() { }
	bool	operator==(const Binning& other) const;
	bool	operator!=(const Binning& other) const;
	bool	operator<(const Binning& other) const;
	virtual std::string	toString() const;
	bool	binned() const { return ((_x > 1) || (_y > 1)); }
};
std::ostream&	operator<<(std::ostream& out, const Binning& binning);
std::istream&	operator>>(std::istream& out, Binning& binning);

ImagePoint	operator*(const ImagePoint& point, const Binning& mode);
ImagePoint	operator/(const ImagePoint& point, const Binning& mode);
ImageSize	operator*(const ImageSize& size, const Binning& mode);
ImageSize	operator/(const ImageSize& size, const Binning& mode);
ImageRectangle	operator*(const ImageRectangle& rect, const Binning& mode);
ImageRectangle	operator/(const ImageRectangle& rect, const Binning& mode);

/**
 * \brief An Image that only has a partial backing store
 *
 * The idea of this type of image is to only bacu up part of an image
 */
template<typename Pixel>
class WindowedImage : public ImageAdapter<Pixel> {
	Image<Pixel>	_backing;
	ImageRectangle	_roi;
	Pixel	_dummy;
public:
	WindowedImage(const ImageSize& size, const ImageRectangle& roi)
		: ImageAdapter<Pixel>(size),
		  _backing(roi.size()), _roi(roi) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "backing has size: %s",
			_backing.size().toString().c_str());
		_dummy = 0;
	}
	virtual Pixel&	writablepixel(int x, int y) {
		if (!_roi.contains(x, y)) {
			return _dummy;
		}
		return _backing.writablepixel(x - _roi.origin().x(),
					y - _roi.origin().y());
	}
	virtual Pixel	pixel(int x, int y) const {
		if (!_roi.contains(x, y)) {
			return 0;
		}
		return _backing.pixel(x - _roi.origin().x(),
			y - _roi.origin().y());
	}
};

/**
 * \brief connected component criterion
 *
 * predicate for pixels to decide whether a pixel should be considered for
 * the connected component of a point.
 */
template<typename Pixel>
class PixelCriterion {
public:
	PixelCriterion() { }
	virtual bool	operator()(const ImagePoint&, const Pixel&) {
		return false;
	}
};

/**
 * \brief Compute the connected component of a point
 *
 * The base class handles image consisting of unsigned char pixels.
 * Pixels belong to the connected component if the value in the
 * image returned by the operator() is 255.
 */
class ConnectedComponentBase {
protected:
	ImagePoint	_point;
	ImageRectangle	_roi;
	void	setupRoi(const ImageRectangle& roi);
	unsigned char	growpixel(ImageAdapter<unsigned char>& image,
				int x, int y) const;
	int	grow(ImageAdapter<unsigned char>& image) const;
public:
	ConnectedComponentBase(const ImagePoint& point);
	ConnectedComponentBase(const ImagePoint& point,
		const ImageRectangle& roi);
	WindowedImage<unsigned char>	*component(
			const ConstImageAdapter<unsigned char>& image);
	static unsigned long	count(
			const ConstImageAdapter<unsigned char>& connected);
	static unsigned long	count(
			const ConstImageAdapter<unsigned char>& connected,
			const ImageRectangle& roi);
};

/**
 * \brief General connected component class for an arbitrarily typed image
 *
 * The _criterion member decides whether points should at all be considered
 * for the connected component, these points then are iteratively grown into
 * a connected component.
 */
template<typename Pixel>
class ConnectedComponent : public ConnectedComponentBase {
	PixelCriterion<Pixel>&	_criterion;
public:
	/**
	 * \brief Constructor, just remembers the defining parameters
 	 */
	ConnectedComponent(const ImagePoint& point,
		PixelCriterion<Pixel>& criterion)
		: ConnectedComponentBase(point), _criterion(criterion) {
	}
	ConnectedComponent(const ImagePoint& point,
		const ImageRectangle& roi,
		PixelCriterion<Pixel>& criterion)
		: ConnectedComponentBase(point, roi), _criterion(criterion) {
	}
	/**
 	 * \brief Compute the connected component
	 */
	WindowedImage<unsigned char>	*operator()(const ConstImageAdapter<Pixel>& image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start CC determination");
		// make sure we have the _roi set
		setupRoi(ImageRectangle(image.getSize()));

		// build an image with unsigned char pixels of the same size
		debug(LOG_DEBUG, DEBUG_LOG, 0, "build standardized image");
		WindowedImage<unsigned char>	*standardized
			= new WindowedImage<unsigned char>(image.getSize(), _roi);
		//standardized->fill(0);

		// initialize the image with 1 for pixels accepted by the
		// criterion and 0 otherwise
		debug(LOG_DEBUG, DEBUG_LOG, 0, "standardize roi pixels");
		int	xmin = _roi.xmin();
		int	ymin = _roi.ymin();
		int	xmax = _roi.xmax();
		int	ymax = _roi.ymax();
		int	counter = 0;
		for (int x = xmin; x < xmax; x++) {
			for (int y = ymin; y < ymax; y++) {
				ImagePoint	p(x, y);
				Pixel	v = image.pixel(x, y);
				if (_criterion(p, v)) {
					standardized->writablepixel(x, y) = 1;
					counter++;
				} else {
					standardized->writablepixel(x, y) = 0;
				}
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "standardized image %d pixels",
			counter);

		//  now use the method of the base class to compute the
		// connected component
		WindowedImage<unsigned char>	*component
			= ConnectedComponentBase::component(*standardized);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy standardized image");
		delete standardized;
		return component;
	}
};

/**
 * \brief Compute a list of all points where the pixel value is maximal
 */
std::list<ImagePoint>	Maxima(ImagePtr image, unsigned long limit = 10);

/**
 * \brief Format reduction to 8 bits so that images can be saved as PNG or JPEG
 */
class FormatReductionBase {
protected:
	double	_min;
	double	_max;
	unsigned char	clamp(double v) const;
public:
	FormatReductionBase(double min, double max);
};

class FormatReduction : public FormatReductionBase,
			public ConstImageAdapter<unsigned char> {
public:
	FormatReduction(const ImageSize& size, double min, double max);
	static FormatReduction	*get(ImagePtr image);
	static FormatReduction	*get(ImagePtr image, double min, double max);
	static FormatReduction	*get(ImagePtr image,
					std::pair<double, double>& minmax);
	static std::pair<double, double>	range(ImagePtr image);
	static std::pair<double, double>	mrange(ImagePtr image);
};

class FormatReductionRGB : public FormatReductionBase,
			public ConstImageAdapter<RGB<unsigned char> > {
public:
	FormatReductionRGB(const ImageSize& size, double min, double max);
	static FormatReductionRGB	*get(ImagePtr image);
	static FormatReductionRGB	*get(ImagePtr image,
					double min, double max);
	static FormatReductionRGB	*get(ImagePtr image,
					std::pair<double, double>& minmax);
	static std::pair<double, double>	range(ImagePtr image);
};

/**
 * \brief Format class as the base class for all special formats
 */
class Format {
public:
	typedef enum type_e { FITS, JPEG, PNG } type_t;
protected:
	type_t	_type;
public:
	type_t	type() const { return _type; }
	Format(type_t type = FITS) : _type(type) { }
	std::string	typeString() const;

	size_t	write(ImagePtr image, const std::string& filename);
	size_t	write(ImagePtr image, type_t type,
			void **buffer, size_t *buffersize);
	ImagePtr	read(type_t type, void *buffer, size_t buffersize);
};

/**
 * \brief Axiliary class to read/write images from/to files and memory buffers
 */
class FITS : public Format {
	size_t	write(ImagePtr image, const std::string& filename);
public:
	static bool	isfitsfilename(const std::string& filename);
	FITS();

	// write images
	size_t	writeFITS(ImagePtr image, const std::string& filename);
	size_t	writeFITS(ImagePtr image, void **buffer, size_t *buffersize);

	// read images
	ImagePtr	readFITS(const std::string& filename);
	ImagePtr	readFITS(void *buffer, size_t buffersize);
};

/**
 * \brief Auxiliary class to read and write JPEG images from/to files/memory
 */
class JPEG : public Format {
	int	_quality;
public:
	static bool	isjpegfilename(const std::string& filename);
	JPEG();
	int	quality() const { return _quality; }
	void	quality(int q) { _quality = q; }

	// basic write operations with 8bit pixel sizes
	size_t	writeJPEG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
			void **buffer, size_t *buffersize);
	size_t	writeJPEG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
			const std::string& filename);
	size_t	writeJPEG(const ConstImageAdapter<unsigned char>& monoimage,
			void **buffer, size_t *buffersize);
	size_t	writeJPEG(const ConstImageAdapter<unsigned char>& monoimage,
			const std::string& filename);

	// write generic image
	size_t	writeJPEG(const ImagePtr image,
			void **buffer, size_t *buffersize);
	size_t	writeJPEG(const ImagePtr image, const std::string& filename);

	// read JPEG images
	ImagePtr	readJPEG(const std::string& filename);
	ImagePtr	readJPEG(void *buffer, size_t buffersize);
};

/**
 * \brief Auxiliary class to read and write PNG images from/to files/memory
 */
class PNG : public Format {
public:
	static bool	ispngfilename(const std::string& filename);

	PNG();

	// basic write operations with 8bit pixel sizes
	size_t	writePNG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
			void **buffer, size_t *buffersize);
	size_t	writePNG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
			const std::string& filename);
	size_t	writePNG(const ConstImageAdapter<unsigned char>& monoimage,
			void **buffer, size_t *buffersize);
	size_t	writePNG(const ConstImageAdapter<unsigned char>& monoimage,
			const std::string& filename);

	// write generic image
	size_t	writePNG(ImagePtr image,
			void **buffer, size_t *buffersize);
	size_t	writePNG(ImagePtr image, const std::string& filename);

	// read PNG images
	ImagePtr	readPNG(const std::string& filename);
	ImagePtr	readPNG(void *buffer, size_t buffersize);
};

/**
 * \brief Container class for images as memory buffers
 */
class ImageBuffer : public Format {
	ImageBuffer&	operator=(const ImageBuffer& other) = delete;
	void	*_buffer;
	size_t	_buffersize;
public:
	size_t	buffersize() const { return _buffersize; }

	ImageBuffer(ImagePtr image);
	ImageBuffer(ImagePtr image, type_t type);
	ImageBuffer(const std::string& filename);
	ImageBuffer(type_t type, void *buffer, size_t buffersize);
	ImageBuffer(const ImageBuffer& other);
	~ImageBuffer();

	ImagePtr	image() const;
	void	write(const std::string& filename) const;
	void	write(void **buffer, size_t *buffersize);
	ImageBuffer	*convert(type_t type) const;
	void	*data() const { return _buffer; }
	size_t	size() const { return _buffersize; }
};

typedef std::shared_ptr<ImageBuffer>	ImageBufferPtr;

} // namespace image
} // namespace astro

#endif /* _AstroImage_h */
