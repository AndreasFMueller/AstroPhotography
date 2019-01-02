/*
 * FITSoutfile.cpp -- implementation of FITS io routines
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <fitsio.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

using namespace astro::image;

namespace astro {
namespace io {

/**
 * \brief Create a FITS file for writing
 */
FITSoutfileBase::FITSoutfileBase(const std::string &filename,
	int pixeltype, int planes, int imgtype) 
	: FITSfile(filename, pixeltype, planes, imgtype) {
	_precious = true;
}

/**
 * \brief write the image format information to the header
 */
void	FITSoutfileBase::write(const ImageBase& image) {
	// if the file exists but is not precious, and writable, unlink it
	struct stat	sb;
	int	rc = stat(filename.c_str(), &sb);
	if (rc == 0) {
		// file exists, check that it is a file
		if (!S_ISREG(sb.st_mode)) {
			std::string	msg = stringprintf("%s is not a file",
				filename.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw FITSexception(msg);
		}

		// check whether the file is precious
		if (precious()) {
			std::string	msg = stringprintf(
				"%s is precious, cannot overwrite",
				filename.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw FITSexception(msg);
		}

		// check whether the file writable
		if (access(filename.c_str(), W_OK) < 0) {
			std::string	msg = stringprintf("%s is not writable",
				filename.c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}

		// unlink the file
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unklink(%s) existing file",
			filename.c_str());
		if (unlink(filename.c_str())) {
			std::string	msg = stringprintf("cannot unlink "
				"%s: %s", filename.c_str(), strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}

	// create the file
	int	status = 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create FITS file %s", filename.c_str());
	if (fits_create_file(&fptr, filename.c_str(), &status)) {
		throw FITSexception(errormsg(status));
	}

	// find the dimensions
	long	naxis = 3;
	long	naxes[3] = {
		image.size().width(), image.size().height(), planes
	};

	status = 0;
	if (fits_create_img(fptr, imgtype, naxis, naxes, &status)) {
		throw FITSexception(errormsg(status), filename);
	}

	// write all the additional headers we would like to have in
	// an image
	ImageMetadata::const_iterator	i;
	for (i = image.begin(); i != image.end(); i++) {
		const char	*key = i->first.c_str();
		Metavalue	value = i->second;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "writing '%s'",
			value.toString().c_str());
		const char	*comment = value.getComment().c_str();
		std::type_index	type = value.getType();
		int	status = 0;
		int	rc = 0;

		if (type == std::type_index(typeid(bool))) {
			int	logicalvalue = (bool)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (bool)%s",
				key, (logicalvalue) ? "true" : "false");
			rc = fits_write_key(fptr, TLOGICAL, key, &logicalvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(std::string))) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (string)%s",
				key, value.getValue().c_str());
			rc = fits_write_key(fptr, TSTRING, key,
				(void *)value.getValue().c_str(),
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(char))) {
			char	charvalue = (char)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (char)%d",
				key, charvalue);
			rc = fits_write_key(fptr, TBYTE, key, &charvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(short))) {
			short	shortvalue = (short)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (short)%d",
				key, shortvalue);
			rc = fits_write_key(fptr, TSHORT, key, &shortvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(unsigned short))) {
			unsigned short	ushortvalue = (unsigned short)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (ushort)%hu",
				key, ushortvalue);
			rc = fits_write_key(fptr, TUSHORT, key, &ushortvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(int))) {
			int	intvalue = (int)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (int)%d",
				key, intvalue);
			rc = fits_write_key(fptr, TINT, key, &intvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(unsigned int))) {
			unsigned int	uintvalue = (unsigned int)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (uint)%u",
				key, uintvalue);
			rc = fits_write_key(fptr, TUINT, key, &uintvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(long))) {
			long	longvalue = (long)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (long)%ld",
				key, longvalue);
			rc = fits_write_key(fptr, TLONG, key, &longvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(unsigned long))) {
			unsigned long	ulongvalue = (unsigned long)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (ulong)%lu",
				key, ulongvalue);
			rc = fits_write_key(fptr, TULONG, key, &ulongvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(float))) {
			float	floatvalue = (float)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (float)%f",
				key, floatvalue);
			rc = fits_write_key(fptr, TFLOAT, key, &floatvalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(double))) {
			double doublevalue = (double)value;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: (double)%f",
				key, doublevalue);
			rc = fits_write_key(fptr, TDOUBLE, key, &doublevalue,
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(FITSdate))) {
			rc = fits_write_key(fptr, TSTRING, key,
				(void *)((std::string)value).c_str(),
				comment, &status);
			goto writedone;
		}

		if (type == std::type_index(typeid(void))) {
			if (key == std::string("HISTORY")) {
				rc = fits_write_history(fptr, comment, &status);
				debug(LOG_DEBUG, DEBUG_LOG, 0, "write HISTORY: %s, %d", comment, rc);
			}
			if (key == std::string("COMMENT")) {
				rc = fits_write_comment(fptr, comment, &status);
			}
			goto writedone;
		}

		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot write entry of type %s",
			type.name());
		continue;
	
	writedone:
		if (rc) {
			throw FITSexception(errormsg(status), filename);
		}
	}
}

/**
 * \brief Fix permissions on precious files
 */
void	FITSoutfileBase::postwrite() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "postwrite called");
	// not precious, do nothing
	if (!precious()) {
		return;
	}

	// find current permissions
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat %s: %s",
			filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw FITSexception(msg, filename);
	}

	// compute and set new permissions
	if (chmod(filename.c_str(), sb.st_mode & (~(S_IWUSR | S_IWGRP | S_IWOTH))) < 0) {
		std::string	msg = stringprintf("cannot chmod %s: %s",
			filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw FITSexception(msg, filename);
	}
}

/**
 * \brief constructor specializations of FITSoutfile for all types
 */
#define FITS_OUT_CONSTRUCTOR(T, pix, planes, img)			\
template<>								\
FITSoutfile<T >::FITSoutfile(const std::string& filename)		\
	: FITSoutfileBase(filename, pix, planes, img) {			\
}

// basic type monochrome pixels
FITS_OUT_CONSTRUCTOR(unsigned char, TBYTE, 1, BYTE_IMG)
FITS_OUT_CONSTRUCTOR(unsigned short, TUSHORT, 1, USHORT_IMG)
FITS_OUT_CONSTRUCTOR(unsigned int, TULONG, 1, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(unsigned long, TULONG, 1, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(float, TFLOAT, 1, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR(double, TDOUBLE, 1, DOUBLE_IMG)

// RGB Pixels
FITS_OUT_CONSTRUCTOR(RGB<unsigned char> , TBYTE, 3, BYTE_IMG)
FITS_OUT_CONSTRUCTOR(RGB<unsigned short> , TUSHORT, 3, USHORT_IMG)
FITS_OUT_CONSTRUCTOR(RGB<unsigned int> , TUINT, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(RGB<unsigned long> , TULONG, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(RGB<float> , TFLOAT, 3, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR(RGB<double> , TDOUBLE, 3, DOUBLE_IMG)

// XYZ Pixels
FITS_OUT_CONSTRUCTOR(XYZ<unsigned char> , TBYTE, 3, BYTE_IMG)
FITS_OUT_CONSTRUCTOR(XYZ<unsigned short> , TUSHORT, 3, USHORT_IMG)
FITS_OUT_CONSTRUCTOR(XYZ<unsigned int> , TUINT, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(XYZ<unsigned long> , TULONG, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(XYZ<float> , TFLOAT, 3, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR(XYZ<double> , TDOUBLE, 3, DOUBLE_IMG)

// YUYV Pixels
FITS_OUT_CONSTRUCTOR(YUYV<unsigned char>, TBYTE, 3, BYTE_IMG)
FITS_OUT_CONSTRUCTOR(YUYV<unsigned short>, TUSHORT, 3, USHORT_IMG)
FITS_OUT_CONSTRUCTOR(YUYV<unsigned int>, TULONG, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(YUYV<unsigned long>, TULONG, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR(YUYV<float>, TFLOAT, 3, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR(YUYV<double>, TDOUBLE, 3, DOUBLE_IMG)

#define FITS_OUT_CONSTRUCTOR_MULTI(T, pix, planes, img)			\
template<>								\
FITSoutfile<Multiplane<T, planes> >::FITSoutfile(const std::string& filename)		\
	: FITSoutfileBase(filename, pix, planes, img) {			\
}

FITS_OUT_CONSTRUCTOR_MULTI(unsigned char, TBYTE, 1, BYTE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned char, TBYTE, 2, BYTE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned char, TBYTE, 3, BYTE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned char, TBYTE, 4, BYTE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned char, TBYTE, 5, BYTE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned char, TBYTE, 6, BYTE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned char, TBYTE, 7, BYTE_IMG)

FITS_OUT_CONSTRUCTOR_MULTI(unsigned short, TUSHORT, 1, USHORT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned short, TUSHORT, 2, USHORT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned short, TUSHORT, 3, USHORT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned short, TUSHORT, 4, USHORT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned short, TUSHORT, 5, USHORT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned short, TUSHORT, 6, USHORT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned short, TUSHORT, 7, USHORT_IMG)

FITS_OUT_CONSTRUCTOR_MULTI(unsigned int, TULONG, 1, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned int, TULONG, 2, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned int, TULONG, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned int, TULONG, 4, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned int, TULONG, 5, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned int, TULONG, 6, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned int, TULONG, 7, ULONG_IMG)

FITS_OUT_CONSTRUCTOR_MULTI(unsigned long, TULONG, 1, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned long, TULONG, 2, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned long, TULONG, 3, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned long, TULONG, 4, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned long, TULONG, 5, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned long, TULONG, 6, ULONG_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(unsigned long, TULONG, 7, ULONG_IMG)

FITS_OUT_CONSTRUCTOR_MULTI(float, TFLOAT, 1, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(float, TFLOAT, 2, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(float, TFLOAT, 3, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(float, TFLOAT, 4, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(float, TFLOAT, 5, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(float, TFLOAT, 6, FLOAT_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(float, TFLOAT, 7, FLOAT_IMG)

FITS_OUT_CONSTRUCTOR_MULTI(double, TDOUBLE, 1, DOUBLE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(double, TDOUBLE, 2, DOUBLE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(double, TDOUBLE, 3, DOUBLE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(double, TDOUBLE, 4, DOUBLE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(double, TDOUBLE, 5, DOUBLE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(double, TDOUBLE, 6, DOUBLE_IMG)
FITS_OUT_CONSTRUCTOR_MULTI(double, TDOUBLE, 7, DOUBLE_IMG)

} // namespace io
} // namespace astro

