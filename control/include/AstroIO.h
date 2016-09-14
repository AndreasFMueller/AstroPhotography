/*
 * AstroIO.h -- classes and functions to perform image IO to/from IO files
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroIO_h
#define _AstroIO_h

#include <fitsio.h>
#include <AstroImage.h>
#include <map>
#include <set>
#include <AstroDebug.h>
#include <AstroFormat.h>

using namespace astro::image;

namespace astro {
namespace io {

/**
 * \brief Exception class for FITS I/O exceptions
 *
 * The FITS I/O classes throw this exception whenever there is a problem
 * opening, reading or writing a FITS file.
 */
class FITSexception : public std::runtime_error {
public:
	FITSexception(const std::string& cause) : std::runtime_error(cause) { }
	FITSexception(const std::string& cause, const std::string& filename);
	FITSexception(const std::string& cause, const std::string& filename,
		int errno);
};

/**
 * \brief structure to abstract the metadata as it is read from the FITS file
 */
class FITShdu {
public:
static	std::string	unquote(const std::string& s);
	std::string	name;
	std::type_index	type;
	std::string	value;
	std::string	comment;
	FITShdu(const std::string& _name, std::type_index _type)
		: name(_name), type(_type) {
	}
};

/**
 * \brief A class grouping some global information about FITS extensions
 */
class FITSKeywords {
public:
// name list
static const std::set<std::string>&	names();
static bool known(const std::string& name);

// get type and comment for a given name
static int	type(const std::string& name);
static std::type_index	index(const std::string& name);
static const std::string&	comment(const std::string& name);
static bool	unique(const std::string& name);

// conversion between FITS type integer and std::type_index
static std::type_index	index(int tp);
static int	type(std::type_index idx);

// factory methods to create 
static Metavalue	meta(const std::string& name, long value,
				const std::string& comment = std::string(""));
static Metavalue	meta(const std::string& name, double value,
				const std::string& comment = std::string(""));
static Metavalue	meta(const std::string& name, const std::string& value,
				const std::string& comment = std::string(""));
static Metavalue	meta(const std::string& name, const FITSdate& value,
				const std::string& comment = std::string(""));
static Metavalue	meta(const FITShdu& hdu);
};

template<typename srctype, typename desttype>
void	copy_metadata(const srctype& src, desttype& dst,
		const std::set<std::string>& names) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copying image metadata");
	for (auto ptr = src.begin(); ptr != src.end(); ptr++) {
		if (names.end() != names.find(ptr->second.getKeyword())) {
			dst.setMetadata(ptr->second);
		}
	}
}

template<typename srctype, typename desttype>
void	copy_metadata(const srctype& src, desttype& dest) {
	copy_metadata<srctype, desttype>(src, dest, FITSKeywords::names());
}

/**
 * \brief FITS file base class
 *
 * The base class contains the shared data and some shared functions,
 * but it cannot be instantiated directly. Only the derived classes
 * can be instantiated, and determine whether the file is opened for
 * reading or writing.
 */
class FITSfile {
protected:
	// header stuff
	typedef	std::list<std::pair<std::string, FITShdu> >	headerlist;
	headerlist	headers;
	headerlist::const_iterator	find(const std::string& name) const;
	headerlist::iterator	find(const std::string& name);
	bool	hasHDU(const std::string& keyword) const;
	const FITShdu&	getHDU(const std::string& keyword) const;

	// file stuff
	std::string	errormsg(int status) const;
	std::string	filename;
	fitsfile	*fptr;
	int	pixeltype;
	int	planes;
	int	imgtype;

	FITSfile(const std::string & filename,
		int _pixeltype, int _planes, int _imgtype);
	virtual ~FITSfile();
public:
	int	getPixeltype() const { return pixeltype; }
	int	getPlanes() const { return planes; }
	int	getImgtype() const { return imgtype; }
	int	getBytesPerPixel() const;
	bool	hasMetadata(const std::string& key) const;
	Metavalue	getMetadata(const std::string& key) const;
	ImageMetadata	getAllMetadata() const;
};

/**
 * \brief Base class for reading files
 *
 * This class adds an attribute for the size of the image. Note that while
 * FITS files can contain several Images, we only use a single image in
 * every file, so in our context, there is a unique size associated with
 * each FITS file we are reading.
 */
class FITSinfileBase : public FITSfile {
protected:
	/**
	 * \brief Size of the image we are about to read
	 */
	void	readkeys() throw (FITSexception);
	ImageSize	size;
	void	*readdata() throw (FITSexception);
protected:
	void	addHeaders(ImageBase *image) const;
public:
	FITSinfileBase(const std::string& filename) throw (FITSexception);
	ImageSize	getSize() const { return size; }
	// header access
	bool	hasHeader(const std::string& key) const;
	std::string	getHeader(const std::string& key) const;
};

/**
 * \brief Open a file and read an image from it
 */
template<typename Pixel>
class FITSinfile : public FITSinfileBase {
public:
	FITSinfile(const std::string& filename) : FITSinfileBase(filename) { }
	Image<Pixel>	*read() throw(FITSexception);
};

/**
 * \brief Convert the pixels read from the FITS file into 
 *
 * This template function doConvertFITSpixels is used to select
 * different algorithms dependending on whether we have RGB, YUYV
 * or monochrome pixels as the target.
 */
template<typename Pixel, typename srctype, typename colortype>
void	doConvertFITSpixels(Pixel *pixels, const srctype *srcpixels,
		int pixelcount, const colortype&) {
	int	size1 = pixelcount;
	int	size2 = pixelcount << 1;
	for (int offset = 0; offset < pixelcount; offset++) {
		RGB<srctype> rgb(	srcpixels[offset],
					srcpixels[offset + size1],
					srcpixels[offset + size2]);
		convertPixel(pixels[offset], rgb);
	}
}

/**
 * \brief Convert to RGB data read from the FITS file to YUYV pixels.
 *
 * This specialization reads RGB data from the FITS file and converts
 * to appropriate YUYV pixels.
 */
template<typename Pixel, typename srctype>
void	doConvertFITSpixels(Pixel *pixels, const srctype *srcpixels, 
		int pixelcount, const yuyv_color_tag) {
	int	size1 = pixelcount;
	int	size2 = pixelcount << 1;
	RGB<srctype>	rgb[2] = { 0, 0 };
	for (int offset = 0; offset < pixelcount; offset += 2) {
		rgb[0].R = srcpixels[offset];
		rgb[0].G = srcpixels[offset + size1],
		rgb[0].G = srcpixels[offset + size2];
		rgb[1].R = srcpixels[offset + 1        ];
		rgb[1].G = srcpixels[offset + 1 + size1],
		rgb[1].G = srcpixels[offset + 1 + size2];
		convertPixelPair(pixels + offset, rgb);
	}
}

/**
 * \brief Convert monochrome image data from the FITS file to the target image
 *        pixel type.
 */
template<typename Pixel, typename srctype>
void	doConvertFITSpixels(Pixel *pixels, const srctype *srcpixels, 
		int pixelcount, const monochrome_color_tag&) {
	convertPixelArray(pixels, srcpixels, pixelcount);
}

/**
 * \brief Convert Multiplane image data from FITS file to the target image
 *        pixel type
 */
template<typename Pixel, typename srctype>
void	doConvertFITSPixels(Pixel *pixels, const srctype *srcpixels,
		int pixelcount, const multiplane_color_tag&) {
	for (int offset = 0; offset < pixelcount; offset++) {
		for (unsigned int i = 0; i < Pixel::planes; i++) {
			pixels[offset].p[i] = srcpixels[offset + i * pixelcount];
		}
	}
}

/**
 * \brief Convert Pixel arrays from a privitive type to any other valid
 *        pixel type.
 *
 * This template function is called by the read method in the
 * FITSinfile<Pixel> template class. Get more information about the
 * rationale for these classes in the description of FITSinfile<Pixel>::read
 */
template<typename Pixel, typename srctype>
void	convertFITSpixels(Pixel *pixels, srctype *srcpixels,
		int pixelcount) {
	doConvertFITSpixels(pixels, srcpixels, pixelcount,
		typename color_traits<Pixel>::color_category());
}

/**
 * \brief Read the data from a FITS file into an Image
 *
 * This method reads the data from the FITS file and converts it
 * into the array of pixels in the image. But the pixel type of the
 * image can be different from the pixel type read from the FITS file.
 * In order to be consistent, we want to apply the same pixel conversions
 * when reading pixels from a file with different type. However, there
 * is a problem: the type returned from the FITS library is not typed,
 * it's a void pointer. So we have to cast it to the appropriate type
 * based on the Image type we got from the FITS file. So we call the
 * convertFITSpixels template function, giving it the value type
 * (from the pixel_value_type traits class. The template selection
 * mechanism then instantiates the right converFITSpixels template
 * function.
 */
template<typename Pixel>
Image<Pixel>	*FITSinfile<Pixel>::read()
	throw (FITSexception) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading FITS file");
	Image<Pixel>	*image = new Image<Pixel>(size);

	// the FITSinfile constructor has already read the header data
	// so we con copy the headers into the metadata now
	addHeaders(image);

	// read the data
	void	*data = readdata();

	// convert to the target pixel type
	switch (imgtype) {
	case BYTE_IMG:
	case SBYTE_IMG:
		convertFITSpixels(image->pixels, (unsigned char *)data,
			image->getSize().getPixels());
		break;
	case USHORT_IMG:
	case SHORT_IMG:
		convertFITSpixels(image->pixels, (unsigned short *)data,
			image->getSize().getPixels());
		break;
	case ULONG_IMG:
	case LONG_IMG:
		convertFITSpixels(image->pixels, (unsigned int *)data,
			image->getSize().getPixels());
		break;
	case FLOAT_IMG:
		convertFITSpixels(image->pixels, (float *)data,
			image->getSize().getPixels());
		break;
	case DOUBLE_IMG:
		convertFITSpixels(image->pixels, (double *)data,
			image->getSize().getPixels());
		break;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading FITS file completed");
	return image;
}


/**
 * \brief Manage a fits output file.
 *
 * This class is used as a base class for writing FITS files.
 * A template class derived from this class can then be used
 * to write the image contents.
 */
class FITSoutfileBase : public FITSfile {
	bool	_precious;
public:
	FITSoutfileBase(const std::string & filename,
		int _pixeltype, int _planes, int _imgtype)
		throw (FITSexception);
	void	write(const ImageBase& image)
			throw (FITSexception);
	void	postwrite() throw (FITSexception);
	bool	precious() const { return _precious; }
	void	setPrecious(bool precious) { _precious = precious; }	
};

/**
 * \brief FITS output file class template to write files of any pixel type
 */
template<class Pixel>
class FITSoutfile : public FITSoutfileBase {
public:
	FITSoutfile(const std::string& filename) throw (FITSexception);
	void	write(const Image<Pixel>& image)
			throw (FITSexception);
};

/**
 * \brief Create a FITS file for writing
 */
template<class Pixel>
FITSoutfile<Pixel>::FITSoutfile(const std::string& filename)
	throw (FITSexception) : FITSoutfileBase(filename, TBYTE, 1, BYTE_IMG) {
}

// Specializations for just about any type. They are all needed, because
// this is where we map the pixel type and image type codes from the
// CFITSIO library to C++ types
#define	FITS_OUTFILE_SPECIALIZATION(T)					\
template<>								\
FITSoutfile<T >::FITSoutfile(const std::string& filename)		\
	throw (FITSexception);

FITS_OUTFILE_SPECIALIZATION(unsigned char)
FITS_OUTFILE_SPECIALIZATION(unsigned short)
FITS_OUTFILE_SPECIALIZATION(unsigned int)
FITS_OUTFILE_SPECIALIZATION(unsigned long)
FITS_OUTFILE_SPECIALIZATION(float)
FITS_OUTFILE_SPECIALIZATION(double)

FITS_OUTFILE_SPECIALIZATION(RGB<unsigned char>)
FITS_OUTFILE_SPECIALIZATION(RGB<unsigned short>)
FITS_OUTFILE_SPECIALIZATION(RGB<unsigned int>)
FITS_OUTFILE_SPECIALIZATION(RGB<unsigned long>)
FITS_OUTFILE_SPECIALIZATION(RGB<float>)
FITS_OUTFILE_SPECIALIZATION(RGB<double>)

FITS_OUTFILE_SPECIALIZATION(YUYV<unsigned char>)
FITS_OUTFILE_SPECIALIZATION(YUYV<unsigned short>)
FITS_OUTFILE_SPECIALIZATION(YUYV<unsigned int>)
FITS_OUTFILE_SPECIALIZATION(YUYV<unsigned long>)
FITS_OUTFILE_SPECIALIZATION(YUYV<float>)
FITS_OUTFILE_SPECIALIZATION(YUYV<double>)

#define	FITS_OUTFILE_SPECIALIZATION_MULTI(T, N)				\
template<>								\
FITSoutfile<Multiplane<T, N> >::FITSoutfile(const std::string& filename)\
	throw (FITSexception);

FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned char, 1)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned short, 1)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned int, 1)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned long, 1)
FITS_OUTFILE_SPECIALIZATION_MULTI(float, 1)
FITS_OUTFILE_SPECIALIZATION_MULTI(double, 1)

FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned char, 2)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned short, 2)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned int, 2)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned long, 2)
FITS_OUTFILE_SPECIALIZATION_MULTI(float, 2)
FITS_OUTFILE_SPECIALIZATION_MULTI(double, 2)

FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned char, 3)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned short, 3)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned int, 3)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned long, 3)
FITS_OUTFILE_SPECIALIZATION_MULTI(float, 3)
FITS_OUTFILE_SPECIALIZATION_MULTI(double, 3)

FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned char, 5)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned short, 5)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned int, 5)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned long, 5)
FITS_OUTFILE_SPECIALIZATION_MULTI(float, 5)
FITS_OUTFILE_SPECIALIZATION_MULTI(double, 5)

FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned char, 6)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned short, 6)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned int, 6)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned long, 6)
FITS_OUTFILE_SPECIALIZATION_MULTI(float, 6)
FITS_OUTFILE_SPECIALIZATION_MULTI(double, 6)

FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned char, 7)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned short, 7)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned int, 7)
FITS_OUTFILE_SPECIALIZATION_MULTI(unsigned long, 7)
FITS_OUTFILE_SPECIALIZATION_MULTI(float, 7)
FITS_OUTFILE_SPECIALIZATION_MULTI(double, 7)


/**
 * \brief Holder class for application specific information during FITS
 *        iterator work
 *
 * A FITSIO iterator performs all the iteration work needed to write data
 * from an Image<Pixel> to a FITS file. This class also implements the
 * work function, which is a static function of the class. The default
 * implementation of the work function just copies the data to the
 * target FITS file.  However, some image types, in particular the
 * YUYVPixel and RGBPixel images, require special treatment, as colors
 * need to be separated. Therefore, spezializations for the workfunctions
 * are provided for these pixel types.
 */
template<typename Pixel, typename colortype>
class IteratorData {
public:
	const Image<Pixel>&	image;
	int	plane;
	IteratorData(const Image<Pixel>& _image)
		: image(_image), plane(0) {
	}
	static int	workfunc(long totaln, long offset,
		long firstn, long nvalues, int narray,
		iteratorCol *data, void *userPointer);
};

/**
 * \brief Template functions for the work function
 *
 * There are three diffent ways to write data to the FITS file. Monochrome
 * Pixels are written as a single image, RGB pixels are written as three
 * planes in one image, and YUYV pixels are first converted to RGB
 * and then written to three planes.
 */
template<typename Pixel>
int	FITSWriteDoWork(const long /* totaln */, long /* offset */,
		long /* firstn */, long nvalues, int /* narray */,
		iteratorCol *data, void *userPointer,
		monochrome_color_tag) {
	// get the data array
	typedef	typename pixel_value_type<Pixel>::value_type	value_type;
	value_type   *array = (value_type *)fits_iter_get_array(data);

	// set th efirst pixel element to 0, because that's how
	// the library learns how nul values are represented
	*array++ = 0;

	// get the user data
        IteratorData<Pixel, monochrome_color_tag>     *user
		= (IteratorData<Pixel, monochrome_color_tag>*)userPointer;

	// copy everything from the image to the array
        std::copy(user->image.pixels, user->image.pixels + nvalues, array);
	return 0;
}

/**
 * \brief Work function to write RGB pixels to the FITS file
 *
 * This algorithm just has to redestribute the color channels from pixels
 * each pixel to the three planes of the FITS file.
 */
template<typename Pixel>
int	FITSWriteDoWork(const long /* totaln */, long /* offset */,
		long /* firstn */, long /* nvalues */, int /* narray */,
		iteratorCol *data, void *userPointer,
		rgb_color_tag) {
	// get the data array, and write a zero into it
	typedef	typename pixel_value_type<Pixel>::value_type	value_type;
	value_type   *array = (value_type *)fits_iter_get_array(data);
	*array++ = 0;

	// get the user data pointer
        IteratorData<Pixel, rgb_color_tag>     *user
		= (IteratorData<Pixel, rgb_color_tag>*)userPointer;

	// iterate through the pixels and convert them to RGB on the fly
	const int	size = user->image.getSize().getPixels();
	const int	size2 = user->image.getSize().getPixels() << 1;
	for (int offset = 0; offset < size; offset++) {
		array[offset        ] = user->image[offset].R;
		array[offset + size ] = user->image[offset].G;
		array[offset + size2] = user->image[offset].B;
	}
	return 0;
}

/**
 * \brief Work function to write YUYV Pixels to the FITS file
 */
template<typename Pixel>
int	FITSWriteDoWork(const long /* totaln */, long /* offset */,
		long /* firstn */, long /* nvalues */, int /* narray */,
		iteratorCol *data, void *userPointer,
		yuyv_color_tag) {
	// get the data array and write a zero into it
	typedef	typename pixel_value_type<Pixel>::value_type	value_type;
	value_type   *array = (value_type *)fits_iter_get_array(data);
	*array++ = 0;

	// get the user data
        IteratorData<Pixel, yuyv_color_tag>     *user
		= (IteratorData<Pixel, yuyv_color_tag>*)userPointer;

	// now iteratate through the pixels and convert them pair by pair
	RGB<value_type>	dest[2];
	const int	size = user->image.getSize().getPixels();
	const int	size2 = user->image.getSize().getPixels() << 1;
	for (int offset = 0; offset < size; offset += 2) {
		// convert the YUYV pixel pair to an RGB pixel pair
		convertPixelPair(dest, user->image.pixels + offset);
		// distribute the rgb pixel values to the three planes
		array[offset            ] = dest[0].R;
		array[offset +     size ] = dest[0].G;
		array[offset +     size2] = dest[0].B;
		array[offset + 1        ] = dest[1].R;
		array[offset + 1 + size ] = dest[1].G;
		array[offset + 1 + size2] = dest[1].B;
	}
	return 0;
}

/**
 * \brief Work function to write Multiplane pixels to the FITS file
 */
template<typename Pixel>
int	FITSWriteDoWork(const long /* totaln */, long /* offset */,
		long /* firstn */, long /* nvalues */, int /* narray */,
		iteratorCol *data, void *userPointer,
		multiplane_color_tag) {
	// get the data array, and write a zero into it
	typedef	typename pixel_value_type<Pixel>::value_type	value_type;
	value_type   *array = (value_type *)fits_iter_get_array(data);
	*array++ = 0;

	// get the user data pointer
        IteratorData<Pixel, multiplane_color_tag>     *user
		= (IteratorData<Pixel, multiplane_color_tag>*)userPointer;

	// iterate through the pixels and copy them to the data array
	const int	size = user->image.getSize().getPixels();
	for (int offset = 0; offset < size; offset++) {
		for (int i = 0; i < Pixel::planes; i++) {
			array[offset + i * size] = user->image[offset].p[i];
		}
	}
	return 0;
}

/**
 * \brief Default work function
 *
 * The default work function copies the pixel values unchanged into the
 * target file.
 */
template<typename Pixel, typename colortype>
int	IteratorData<Pixel, colortype>::workfunc(const long totaln, long offset,
		long firstn, long nvalues, int narray,
		iteratorCol *data, void *userPointer) {
	return FITSWriteDoWork<Pixel>(totaln, offset, firstn, nvalues,
		narray, data, userPointer, colortype());
}

/**
 * \brief FITS file write driver.
 *
 * Write an image to a FITS file. This method uses the iterator framework
 * from the CFITSIO library to write each plane separately. For monochrome
 * images, there is only one call to the work function. For color images,
 * the work function is called three times. On each call a different
 * color plane is extracted and sent to the FITS file.
 */
template<typename Pixel>
void	FITSoutfile<Pixel>::write(const Image<Pixel>& image)
	throw (FITSexception) {
	// create the header
	FITSoutfileBase::write(image);

	// iterator control
	iteratorCol	ic;
	fits_iter_set_file(&ic, fptr);
	fits_iter_set_datatype(&ic, pixeltype);
	fits_iter_set_iotype(&ic, OutputCol);

	// prepare the IteratorData structure, which is handed into the
	// iterator work function as user data
	IteratorData<Pixel, typename color_traits<Pixel>::color_category >	
		user(image);
	int	status = 0;
	if (fits_iterate_data(1, &ic, 0, image.getSize().getPixels() * planes,
		user.workfunc, &user, &status)) {
		std::string	msg = stringprintf("failure to write image %s: %s",
			filename.c_str(), errormsg(status).c_str());
		throw FITSexception(msg);
	}

	// flush the file
	if (fits_flush_file(fptr, &status)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flushing file failed: %s",
			errormsg(status).c_str());
	}

	// call postwrite to protect precious files
	postwrite();
}

/**
 * \brief Write a generic image as a FITS file.
 *
 * The ImagePtr is independent of the pixel type. This class can determine
 * the pixel type and use an appropriate FITSoutfile<Pixel> instance to
 * write the image.
 */
class FITSout {
	std::string	filename;
	bool	_precious;
public:
	FITSout(const std::string& filename);
	bool	exists() const;
	void	unlink();
	bool	precious() const { return _precious; }
	void	setPrecious(bool precious) { _precious = precious; }
	void	write(const ImagePtr image) throw (FITSexception);
};

/**
 * \brief Read a generic image as a FITS file
 *
 * Read the image file and create an appropriate Image<P> object, then
 * wrap it in an ImagePtr.
 */
class FITSin {
	std::string	filename;
public:
	FITSin(const std::string& filename);
	ImagePtr	read() throw (FITSexception);
};

/**
 * \brief Image directory
 */
class FITSdirectory {
public:
	typedef enum { COUNTER, TIMESTAMP, BOTH } filenameformat;
private:
	std::string	_path;
	std::string	indexfile;
	std::string	_prefix;
	filenameformat	_format;
	std::string	_timestampformat;
	void	setup();
public:
	FITSdirectory(filenameformat format = COUNTER);
	FITSdirectory(const std::string& path, filenameformat format = COUNTER);
	FITSdirectory(const std::string& prefix, const time_t when,
		filenameformat format = COUNTER);
	// accessors
	const std::string&	prefix() const { return _prefix; }
	void	prefix(const std::string& prefix) { _prefix = prefix; }
	const std::string&	timestampformat() const {
		return _timestampformat;
	}
	void	timestampformat(const std::string& timestampformat) {
		_timestampformat = timestampformat;
	}
	const std::string&	path() const { return _path; }
	// add an image
	std::string	add(const ImagePtr image);
};

} // namespace io
} // namespace astro

#endif /* _AstroIO_h */
