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

	FITSfile(const std::string& _filename);
	virtual ~FITSfile() throw (FITSexception);
public:
};

class FITSinfile : public FITSfile {
public:
	FITSinfile(const std::string& filename) throw (FITSexception);
	astro::image::ImageBase	read() throw (FITSexception);
};

/**
 * \brief Manage a fits output file.
 *
 * This class is used as a base class for writing FITS files.
 * A template class derived from this class can then be used
 * to write the image contents.
 */
class FITSoutfileBase : public FITSfile {
	virtual int	pixeltype() const = 0;
	virtual int	imgtype() const = 0;
	virtual int	planes() const = 0;
public:
	FITSoutfileBase(const std::string & filename) throw (FITSexception);;
	void	write(const astro::image::ImageBase& image)
			throw (FITSexception);
};

/**
 * \brief FITS output file class template to write files of any pixel type
 */
template<class Pixel>
class FITSoutfile : public FITSoutfileBase {
	virtual int	pixeltype() const;
	virtual int	imgtype() const;
	virtual int	planes() const;
public:
	FITSoutfile(const std::string& filename) throw (FITSexception);
	void	write(const astro::image::Image<Pixel>& image)
			throw (FITSexception);
};

/**
 * \brief Create a FITS file for writing
 */
template<class Pixel>
FITSoutfile<Pixel>::FITSoutfile(const std::string& filename)
	throw (FITSexception) : FITSoutfileBase(filename) {
}

/**
 * \brief Default implementation of imgtype assumes byte sized pixels
 */
template<class Pixel>
int	FITSoutfile<Pixel>::imgtype() const { return BYTE_IMG; }

template<> int FITSoutfile<unsigned char>::imgtype() const;
template<> int FITSoutfile<char>::imgtype() const;

template<> int FITSoutfile<unsigned short>::imgtype() const;
template<> int FITSoutfile<short>::imgtype() const;

template<> int FITSoutfile<unsigned int>::imgtype() const;
template<> int FITSoutfile<int>::imgtype() const;

template<> int FITSoutfile<unsigned long>::imgtype() const;
template<> int FITSoutfile<long>::imgtype() const;

template<> int FITSoutfile<float>::imgtype() const;
template<> int FITSoutfile<double>::imgtype() const;

template<class Pixel>
int	FITSoutfile<Pixel>::pixeltype() const { return TBYTE; }

template<> int FITSoutfile<unsigned char>::pixeltype() const;
template<> int FITSoutfile<char>::pixeltype() const;

template<> int FITSoutfile<unsigned short>::pixeltype() const;
template<> int FITSoutfile<short>::pixeltype() const;

template<> int FITSoutfile<unsigned int>::pixeltype() const;
template<> int FITSoutfile<int>::pixeltype() const;

template<> int FITSoutfile<unsigned long>::pixeltype() const;
template<> int FITSoutfile<long>::pixeltype() const;

template<> int FITSoutfile<float>::pixeltype() const;
template<> int FITSoutfile<double>::pixeltype() const;

template<class Pixel>
int	FITSoutfile<Pixel>::planes() const { return 1; }

template<> int FITSoutfile<astro::image::RGBPixel>::planes() const { return 3; }
template<> int FITSoutfile<astro::image::YUYVPixel>::planes() const { return 3; }

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
template<class Pixel>
class IteratorData {
public:
	const astro::image::Image<Pixel>&	image;
	int	plane;
	IteratorData(const astro::image::Image<Pixel>& _image)
		: image(_image), plane(0) {
	}
	static int	workfunc(long totaln, long offset,
		long firstn, long nvalues, int narray,
		iteratorCol *data, void *userPointer);
};

/**
 * \brief Default work function
 *
 * The default work function copies the pixel values unchanged into the
 * target file.
 */
template<class Pixel>
int	IteratorData<Pixel>::workfunc(const long totaln, long offset,
		long firstn, long nvalues, int narray,
		iteratorCol *data, void *userPointer) {
	Pixel   *array = (Pixel *)fits_iter_get_array(data);
        IteratorData<Pixel>     *user
                = (IteratorData<Pixel>*)userPointer;
        std::copy(user->image.pixels, user->image.pixels + nvalues,
                array);
	return 0;
}

template<>
int	IteratorData<astro::image::YUYVPixel>::workfunc(const long totaln,
	long offset, long firstn, long nvalues, int narray,
	iteratorCol *data, void *userPointer);
template<>
int	IteratorData<astro::image::RGBPixel>::workfunc(const long totaln,
	long offset, long firstn, long nvalues, int narray,
	iteratorCol *data, void *userPointer);

/**
 * \brief FITS file write driver.
 *
 * Write an image to a FITS file. This method uses the iterator framework
 * from the CFITSIO library to write each plane separately. For monochrome
 * images, there is only one call to the work function. For color images,
 * the work function is called three times. On each call a different
 * color plane is extracted and sent to the FITS file.
 */
template<class Pixel>
void	FITSoutfile<Pixel>::write(const astro::image::Image<Pixel>& image)
	throw (FITSexception) {
	// create the header
	FITSoutfileBase::write(image);

	// iterator control
	iteratorCol	ic;
	fits_iter_set_file(&ic, fptr);
	fits_iter_set_datatype(&ic, this->pixeltype());
	fits_iter_set_iotype(&ic, OutputCol);

	// prepare the IteratorData structure, which is handed into the
	// iterator work function as user data
	IteratorData<Pixel>	user(image);
	int	status = 0;
std::cerr << "pixels = " << image.size.pixels << std::endl;
	if (fits_iterate_data(1, &ic, 0, image.size.pixels,
		user.workfunc, &user, &status)) {
std::cerr << "fits_iterate_data failed" << std::endl;
		throw FITSexception(errormsg(status));
	}
}

} // namespace io
} // namespace astro

#endif /* _AstroIO_h */
