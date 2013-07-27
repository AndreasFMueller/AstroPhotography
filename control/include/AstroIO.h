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
#include <AstroDebug.h>

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
};

/**
 * \brief structure to abstract 
 */
struct FITShdu {
	std::string	name;
	std::string	value;
	std::string	comment;
	int	type;
};

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
	std::map<std::string, FITShdu>	headers;
	std::string	errormsg(int status) const;
	std::string	filename;
	fitsfile	*fptr;
	int	pixeltype;
	int	planes;
	int	imgtype;

	FITSfile(const std::string & filename,
		int _pixeltype, int _planes, int _imgtype);
	virtual ~FITSfile() throw (FITSexception);
public:
	int	getPixeltype() const { return pixeltype; }
	int	getPlanes() const { return planes; }
	int	getImgtype() const { return imgtype; }
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
	RGB<srctype>	rgb[2];
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
public:
	FITSoutfileBase(const std::string & filename,
		int _pixeltype, int _planes, int _imgtype)
		throw (FITSexception);
	void	write(const ImageBase& image)
			throw (FITSexception);
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
int	FITSWriteDoWork(const long totaln, long offset,
		long firstn, long nvalues, int narray,
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
int	FITSWriteDoWork(const long totaln, long offset,
		long firstn, long nvalues, int narray,
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
int	FITSWriteDoWork(const long totaln, long offset,
		long firstn, long nvalues, int narray,
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
		throw FITSexception(errormsg(status));
	}
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
public:
	FITSout(const std::string& filename);
	void	write(const ImagePtr& image) throw (FITSexception);
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
	std::string	path;
	std::string	indexfile;
	void	setup();
public:
	FITSdirectory();
	FITSdirectory(const std::string& path);
	void	add(const ImagePtr& image);
};

} // namespace io
} // namespace astro

#endif /* _AstroIO_h */
