/*
 * FITSinfile.cpp -- implementation of FITS input routines
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
 * \brief Open a FITS file for reading
 *
 * \param filename	name of the file to read the image from
 */
FITSinfileBase::FITSinfileBase(const std::string& filename)
	: FITSfile(filename, 0, 0, 0) {
	int	status = 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open FITS file '%s'",
		filename.c_str());
	if (fits_open_file(&fptr, filename.c_str(), READONLY, &status)) {
		throw FITSexception(errormsg(status), filename);
	}

	/* read the dimensions of the image from the file */
	int	naxis;
	long	naxes[3];
	int	igt;
	if (fits_get_img_param(fptr, 3, &igt, &naxis, naxes, &status)) {
		throw FITSexception(errormsg(status));
	}
	imgtype = igt;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "params read: imgtype = %d", imgtype);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "             naxis = %d", naxis);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "             naxes[] = [%d,%d,%d]",
		naxes[0], naxes[1], naxes[2]);
	switch (naxis) {
	case 2:
		planes = 1;
		break;
	case 3:
		planes = naxes[2];
		break;
	default:
		throw FITSexception("don't know what to do with image of "
			"dimension != 2 or 3");
	}
	size = ImageSize(naxes[0], naxes[1]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "planes: %d", planes);
	
	switch (planes) {
	case 1:
	case 3:
		break;
//	default:
//		throw std::runtime_error("not 1 or 3 planes");
	}

	// now read the keys
	readkeys();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FITSinfileBase constructor complete");
}

/**
 * \brief Read the raw data
 */
void	*FITSinfileBase::readdata() {
	int	typesize = 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading an image with image type %d",
		imgtype);
	switch (imgtype) {
	case BYTE_IMG:
	case SBYTE_IMG:
			typesize = sizeof(char);
			pixeltype = TBYTE;
			break;
	case USHORT_IMG:
	case SHORT_IMG:
			typesize = sizeof(short);
			pixeltype = TUSHORT;
			break;
	case ULONG_IMG:
	case LONG_IMG:
			typesize = sizeof(long);
			pixeltype = TULONG;
			break;
	case FLOAT_IMG:
			typesize = sizeof(float);
			pixeltype = TFLOAT;
			break;
	case DOUBLE_IMG:
			typesize = sizeof(double);
			pixeltype = TDOUBLE;
			break;
	default:
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown pixel type %d", imgtype);
		throw FITSexception("cannot read this pixel type");
		break;
	}
	void	*v = calloc(planes * size.getPixels(), typesize);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "alloc data size: %d items of size %d, "
		"pixel type %d, %d planes", (size.getPixels() * planes),
		typesize, pixeltype, planes);
	
	/* now read the data */
	int	status = 0;
	long	firstpixel[3] = { 1, 1, 1 };
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading FITS data: pixeltype = %d, "
		"pixels = %d, planes = %d", pixeltype, size.getPixels(),
		planes);
	if (fits_read_pix(fptr, pixeltype, firstpixel,
		size.getPixels() * planes, NULL, v, NULL, &status)) {
		free(v);
		throw FITSexception(errormsg(status));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fits data read: %p", v);
	return v;
}

#define	IGNORED_KEYWORDS_N	8
const char	*ignored_keywords[IGNORED_KEYWORDS_N] = {
	"SIMPLE", "BITPIX", "PCOUNT", "GCOUNT",
	"XTENSION", "END", "BSCALE", "BZERO",
};

/**
 * \brief Find out whether a key should be ignored
 *
 * The read/write functions for the key value pairs in the FITS headers
 * only process headers that are not explicitely handled by the FITS library.
 * Otherwise it would be impossible to keep the headers consistent.
 * This function tells whether a header is ignored, based on the name.
 * It uses the list of ignored_keywords defined above.
 * \param keyname	header key name 
 */
static bool	ignored(const std::string& keyname) {
	if (keyname.substr(0, 5) == "NAXIS") {
		return true;
	}
	for (int i = 0; i < IGNORED_KEYWORDS_N; i++) {
		if (keyname == ignored_keywords[i]) {
			return true;
		}
	}
	return false;
}

#define	STANDARD_HEADER1 "  FITS (Flexible Image Transport System) format is defined in 'Astronomy"
#define STANDARD_HEADER2 "  and Astrophysics', volume 376, page 359; bibcode: 2001A&A...376..359H"

static bool	matches(const std::string& a, const std::string& b) {
	return trim(a) == trim(b);
}

/**
 * \brief Find out whether this is a standard comment header
 *
 * The standard comment headers should not be read, because they are rewritten
 * each time a FITS file is written.
 *
 * \brief hdu	Header data unit containing the header
 */
static bool	isStandardComment(const FITShdu& hdu) {
	return matches(hdu.comment, std::string(STANDARD_HEADER1)) ||
		matches(hdu.comment, std::string(STANDARD_HEADER2));
}

/**
 * \brief Read the headers from a FITS file
 *
 * In the headers we only record the headers that are not managed by
 * the type stuff. I.e. the keywords SIMPLE, BITPIX, NAXIS, NAXISn, END,
 * PCOUNT, GCOUNT, XTENSION are ignored, as defined in the ignored
 * function.
 */
void	FITSinfileBase::readkeys() {
	int	status = 0;
	int	keynum = 1;
	char	keyname[100];
	char	value[100];
	char	comment[100];
	while (1) {
		if (fits_read_keyn(fptr, keynum, keyname, value, comment,
			&status)) {
			// we are at the end of the headers, so we return
			break;
		}
		std::string	name(keyname);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "key[%d] '%s' found", keynum,
			name.c_str());
		if (name.size() == 0) {
			// an empty key means that we are at the end of the
			// list of attributes, so we stop at this point
			break;
		}
		if (ignored(name)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "header '%s' ignored",
				name.c_str());
		} else {
			FITShdu	hdu(name, FITSKeywords::index(name));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "type %s hdu",
				hdu.type.name());
			hdu.comment = comment;
			hdu.value = FITShdu::unquote(value);
			if (isStandardComment(hdu)) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"ignoring standard comment");
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "%s = %s/%s",
					hdu.name.c_str(), hdu.value.c_str(),
					hdu.comment.c_str());
				headers.push_back(make_pair(hdu.name, hdu));
			}
		}
		keynum++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d headers read (headers.size() = %d)",
		keynum - 1, headers.size());
}

/**
 * \brief Copy the headers read from the FITS file into the Image metadata
 */
void	FITSinfileBase::addHeaders(ImageBase *image) const {
	ImageMetadata	metadata = getAllMetadata();
	copy_metadata(metadata, *image);
}

bool	FITSinfileBase::hasHeader(const std::string& key) const {
	return hasHDU(key);
}

std::string	FITSinfileBase::getHeader(const std::string& key) const {
	return getHDU(key).value;
}

} // namespace io
} // namespace astro

