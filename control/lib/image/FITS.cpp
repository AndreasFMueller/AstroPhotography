/*
 * Fits.cpp -- implementation of FITS io routines
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
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
 * \brief remove quotation marks from a string if present
 */
std::string	FITShdu::unquote(const std::string& s) {
	if ((s[0] == '\'') && (s[s.size() - 1] == '\'')) {
		return s.substr(1, s.size() - 2);
	}
	return s;
}

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
FITSfile::FITSfile(const std::string& _filename,
	int _pixeltype, int _planes, int _imgtype)
	: filename(_filename), fptr(NULL), pixeltype(_pixeltype),
	  planes(_planes), imgtype(_imgtype) {
}

/**
 * \brief Destroy a FITS file.
 *
 * This destructor closes the file, if it is open
 */
FITSfile::~FITSfile() {
	if (NULL == fptr) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: no FITS fptr to close",
			filename.c_str());
		return;
	}
	int	status = 0;
	if (fits_close_file(fptr, &status)) {
		// XXX what do I do if the close fails?
		// throw FITSexception(errormsg(status));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "close FITS file %s",
		filename.c_str());
	fptr = NULL;
}

/**
 * \brief Auxiliary predicate class to find headers
 */
class match_header_name {
	std::string	_name;
public:
	match_header_name(const std::string& name) : _name(name) { }
	bool	operator()(const std::pair<std::string, FITShdu>& v) const {
		return v.first == _name;
	}
};

/**
 * \brief Header access
 */
FITSfile::headerlist::const_iterator	FITSfile::find(const std::string& name) const {
	return std::find_if(headers.begin(), headers.end(),
		match_header_name(name));
}

FITSfile::headerlist::iterator	FITSfile::find(const std::string& name) {
	return std::find_if(headers.begin(), headers.end(),
		match_header_name(name));
}

bool	FITSfile::hasHDU(const std::string& keyword) const {
	return find(keyword) != headers.end();
}

const FITShdu&	FITSfile::getHDU(const std::string& keyword) const {
	if (!hasHDU(keyword)) {
		std::string	msg = stringprintf("no header with keyword %s",
			keyword.c_str());
		throw std::runtime_error(msg);
	}
	return find(keyword)->second;
}

/**
 * \brief metadata access
 */
bool	FITSfile::hasMetadata(const std::string& keyword) const {
	return hasHDU(keyword);
}

Metavalue	FITSfile::getMetadata(const std::string& keyword) const {
	return FITSKeywords::meta(getHDU(keyword));
}

ImageMetadata	FITSfile::getAllMetadata() const {
	ImageMetadata	meta;
	headerlist::const_iterator	hi;
	for (hi = headers.begin(); hi != headers.end(); hi++) {
		meta.setMetadata(FITSKeywords::meta(hi->second));
	}
	return meta;
}

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "data size: %d items of size %d, "
		"pixel type %d, %d planes", (size.getPixels() * planes), typesize,
		pixeltype, planes);
	
	/* now read the data */
	int	status = 0;
	long	firstpixel[3] = { 1, 1, 1 };
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading FITS data: pixeltype = %d, "
		"pixels = %d, planes = %d", pixeltype, size.getPixels(),
		planes);
	if (fits_read_pix(fptr, pixeltype, firstpixel, size.getPixels() * planes,
		NULL, v, NULL, &status)) {
		free(v);
		throw FITSexception(errormsg(status));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fits data read");
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

