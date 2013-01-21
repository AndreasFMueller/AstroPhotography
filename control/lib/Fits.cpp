/*
 * Fits.cpp -- implementation of FITS io routines
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroIO.h>
#include <fitsio.h>

using namespace astro::image;

namespace astro {
namespace io {

/**
 * \brief Retrieve a human readable error message from the fits library
 */
std::string	FITSfile::errormsg(int status) const {
	char	errmsg[128];
	fits_get_errstatus(status, errmsg);
	return std::string(errmsg);
}

/**
 * \brief Construct a FITS file object
 *
 * This does not open a file, which is reserved to the derived classes.
 */
FITSfile::FITSfile(const std::string& _filename)
	: filename(_filename), fptr(NULL) {
}

/**
 * \brief Destroy a FITS file.
 *
 * This destructor closes the file, if it is open
 */
FITSfile::~FITSfile() throw (FITSexception) {
	if (NULL == fptr) {
		return;
	}
	int	status = 0;
	if (fits_close_file(fptr, &status)) {
		throw FITSexception(errormsg(status));
	}
	fptr = NULL;
}

/**
 * \brief Open a FITS file for reading
 */
FITSinfile::FITSinfile(const std::string& filename) throw (FITSexception)
	: FITSfile(filename) {
	int	status = 0;
	if (fits_open_file(&fptr, filename.c_str(), READONLY, &status)) {
		throw FITSexception(errormsg(status));
	}
}

/**
 * \brief Read an image 
 */
ImageBase	FITSinfile::read() throw (FITSexception) {
	// first find the type 
}

/**
 * \brief Create a FITS file for writing
 */
FITSoutfileBase::FITSoutfileBase(const std::string &filename)
	throw (FITSexception) : FITSfile(filename) {
	int	status = 0;
	if (fits_create_file(&fptr, filename.c_str(), &status)) {
		throw FITSexception(errormsg(status));
	}
}

/**
 * \brief Pixel type code to write
 *
 * When preparing to write an image, we  need to know the type of the
 * pixels we want to write. This method should be overwritten by
 * specializations in derived template classes so that we get the
 * correct CFITSIO pixel type code.
 */
int	FITSoutfileBase::pixeltype() const {
	return TBYTE;
}

/**
 * \brief Number of planes to write
 */
int	FITSoutfileBase::planes() const {
	return 1;
}

/**
 * \brief write the image format information to the header
 */
void	FITSoutfileBase::write(const ImageBase& image) throw (FITSexception) {
	/* find the type to write */
	int	type = this->imgtype();

	long	naxis = 3;
	long	naxes[3] = { image.size.width, image.size.height,
				this->planes() };
	int	status = 0;
	if (fits_create_img(fptr, type, naxis, naxes, &status)) {
		throw FITSexception(errormsg(status));
	}
}

/*
 * we don't want to have the CFITSIO codes in the header files, so we
 * implement the spezializations here.
 */
template<>
int     FITSoutfile<unsigned char>::imgtype() const { return BYTE_IMG; }
template<>
int     FITSoutfile<char>::imgtype() const { return SBYTE_IMG; }

template<>
int     FITSoutfile<unsigned short>::imgtype() const { return USHORT_IMG; }
template<>
int     FITSoutfile<short>::imgtype() const { return SHORT_IMG; }

template<>
int     FITSoutfile<unsigned int>::imgtype() const { return ULONG_IMG; }
template<>
int     FITSoutfile<int>::imgtype() const { return LONG_IMG; }

template<>
int     FITSoutfile<unsigned long>::imgtype() const { return ULONG_IMG; }
template<>
int     FITSoutfile<long>::imgtype() const { return LONG_IMG; }

template<>
int     FITSoutfile<float>::imgtype() const { return FLOAT_IMG; }
template<>
int     FITSoutfile<double>::imgtype() const { return DOUBLE_IMG; }

/* now the pixel type specializations */
template<>
int     FITSoutfile<unsigned char>::pixeltype() const { return TBYTE; }
template<>
int     FITSoutfile<char>::pixeltype() const { return TSBYTE; }

template<>
int     FITSoutfile<unsigned short>::pixeltype() const { return TUSHORT; }
template<>
int     FITSoutfile<short>::pixeltype() const { return TSHORT; }

template<>
int     FITSoutfile<unsigned int>::pixeltype() const { return TUINT; }
template<>
int     FITSoutfile<int>::pixeltype() const { return TINT; }

template<>
int     FITSoutfile<unsigned long>::pixeltype() const { return TULONG; }
template<>
int     FITSoutfile<long>::pixeltype() const { return TLONG; }

template<>
int     FITSoutfile<float>::pixeltype() const { return TFLOAT; }
template<>
int     FITSoutfile<double>::pixeltype() const { return TDOUBLE; }

/*
 * next we declare the specializations for for the number of planes
 */


} // namespace io
} // namespace astro
