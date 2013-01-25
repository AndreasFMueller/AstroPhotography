/*
 * AstroIO.h -- classes and functions to perform image IO to/from IO
 *                files
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroIO_h
#define _AstroIO_h

#include <fitsio.h>
#include <AstroImage.h>

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
 * \brief FITS file base class
 *
 * The base class contains the shared data and some shared functions,
 * but it cannot be instantiated directly. Only the derived classes
 * can be instantiated, and determine whether the file is opened for
 * reading or writing.
 */
class FITSfile {
protected:
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
 */
class FITSinfileBase : public FITSfile {
protected:
	ImageSize	size;
	void	*readdata() throw (FITSexception);
public:
	FITSinfileBase(const std::string& filename) throw (FITSexception);
	ImageSize	getSize() const { return size; }
};

/**
 * \brief Open a file and read an image from it
 */
template<typename Pixel>
class FITSinfile : public FITSinfileBase {
protected:
public:
	FITSinfile(const std::string& filename) : FITSinfileBase(filename) { }
	std::tr1::shared_ptr<Image<Pixel> >	read() throw (FITSexception);
};

/**
 * \brief Convert the pixels read from the FITS file into 
 */
template<typename Pixel, typename srctype, typename colortype>
void	doConvertFITSpixels(Pixel *pixels, const srctype *srcpixels,
		int pixelcount, const colortype&) {
	std::cerr << "RGB pixel conversion: pixelcount = " << pixelcount << std::endl;
	int	size1 = pixelcount;
	int	size2 = pixelcount << 1;
	for (int offset = 0; offset < pixelcount; offset++) {
		RGB<srctype> rgb(	srcpixels[offset],
					srcpixels[offset + size1],
					srcpixels[offset + size2]);
		convertPixel(pixels[offset], rgb);
	}
}

template<typename Pixel, typename srctype>
void	doConvertFITSpixels(Pixel *pixels, const srctype *srcpixels, 
		int pixelcount, const yuyv_color_tag) {
	std::cerr << "RGB pixel conversion: pixelcount = "
		<< pixelcount << std::endl;
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

template<typename Pixel, typename srctype>
void	doConvertFITSpixels(Pixel *pixels, const srctype *srcpixels, 
		int pixelcount, const monochrome_color_tag&) {
	std::cerr << "convert monochrome pixels" << std::endl;
	convertPixelArray(pixels, srcpixels, pixelcount);
}

template<typename Pixel, typename srctype>
void	convertFITSpixels(Pixel *pixels, srctype *srcpixels,
		int pixelcount) {
std::cerr << "converting pixels" << std::endl;
	doConvertFITSpixels(pixels, srcpixels, pixelcount,
		typename color_traits<Pixel>::color_category());
}

/**
 * \brief Copy void data, but in a type safe way, and in a way that
 *        conversions are only instantiated if the pixel type is
 *        matches the image pixel type.
 */
template<typename Pixel, typename pixel_value_type, typename src_value_type>
void	convertPixelsFromVoid(Pixel *pixels, src_value_type *data,
	int pixelcount, const pixel_value_type&) {
	
}
#define	CONVERT_FROM_VOID(P, S)					\
template<typename P>						\
void	convertPixelsFromVoid(P *pixels, S *data,		\
	int pixelcount,						\
	const S&) {						\
	convertFITSpixels(pixels, data, pixelcount);		\
}
CONVERT_FROM_VOID(Pixel, unsigned char)
CONVERT_FROM_VOID(Pixel, unsigned short)
CONVERT_FROM_VOID(Pixel, unsigned int)
CONVERT_FROM_VOID(Pixel, unsigned long)
CONVERT_FROM_VOID(Pixel, float)
CONVERT_FROM_VOID(Pixel, double)

/**
 * \brief Read the data from a FITS file into an Image
 */
template<typename Pixel>
std::tr1::shared_ptr<Image<Pixel> >	FITSinfile<Pixel>::read()
	throw (FITSexception) {
	Image<Pixel>	*image = new Image<Pixel>(size);
	// read the data
	void	*data = readdata();

	// convert to the target pixel type
	switch (imgtype) {
	case BYTE_IMG:
	case SBYTE_IMG:
		convertPixelsFromVoid(image->pixels, (unsigned char *)data,
			image->size.pixels,
			typename pixel_value_type<Pixel>::value_type());
		break;
	case USHORT_IMG:
	case SHORT_IMG:
		convertPixelsFromVoid(image->pixels, (unsigned short *)data,
			image->size.pixels,
			typename pixel_value_type<Pixel>::value_type());
		break;
	case ULONG_IMG:
	case LONG_IMG:
		convertPixelsFromVoid(image->pixels, (unsigned long *)data,
			image->size.pixels,
			typename pixel_value_type<Pixel>::value_type());
		break;
	case FLOAT_IMG:
		convertPixelsFromVoid(image->pixels, (float *)data,
			image->size.pixels,
			typename pixel_value_type<Pixel>::value_type());
		break;
	case DOUBLE_IMG:
		convertPixelsFromVoid(image->pixels, (double *)data,
			image->size.pixels,
			typename pixel_value_type<Pixel>::value_type());
		break;
	}

	return std::tr1::shared_ptr<Image<Pixel> >(image);
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

// specializations for just about any type
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
	const int	size = user->image.size.pixels;
	const int	size2 = user->image.size.pixels << 1;
	for (int offset = 0; offset < size; offset++) {
		array[offset        ] = user->image[offset].R;
		array[offset + size ] = user->image[offset].G;
		array[offset + size2] = user->image[offset].B;
	}
	return 0;
}

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
	const int	size = user->image.size.pixels;
	const	int	size2 = user->image.size.pixels << 1;
	for (int offset = 0; offset < size; offset += 2) {
		convertPixelPair(dest, user->image.pixels + offset);
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
	if (fits_iterate_data(1, &ic, 0, image.size.pixels * planes,
		user.workfunc, &user, &status)) {
		throw FITSexception(errormsg(status));
	}
}

} // namespace io
} // namespace astro

#endif /* _AstroIO_h */
