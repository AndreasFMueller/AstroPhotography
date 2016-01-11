/*
 * FITSKeywords.cpp -- implementation of type mapping functions for attributes
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <mutex>
#include <list>

namespace astro {
namespace io {

typedef struct {
	std::string	name;
	std::string	comment;
	std::type_index	index;
	bool	unique;
} FITSKeyword;

#define	Nkeywors	78
FITSKeyword	keywors[Nkeywors] = {
// standard keywords
{ // 0
	std::string("AUTHOR"),
	std::string("author of the data"),
	std::type_index(typeid(std::string)),
	false
},
{ // 1
	std::string("BITPIX"),
	std::string("bits per data value"),
	std::type_index(typeid(long)),
	true
},
{ // 2
	std::string("BLANK"),
	std::string("value used for undefined array elements"),
	std::type_index(typeid(long)),
	true
},
{ // 3
	std::string("BLOCKED"),
	std::string("is physical blocksize a multiple of 2880"),
	std::type_index(typeid(bool)),
	true
},
{ // 4
	std::string("BSCALE"),
	std::string("linear factor in scaling equation"),
	std::type_index(typeid(double)),
	true
},
{ // 5
	std::string("BUNIT"),
	std::string("physical units of the array values"),
	std::type_index(typeid(std::string)),
	true
},
{ // 6
	std::string("BZERO"),
	std::string("zero point in scaling equation"),
	std::type_index(typeid(double)),
	true
},
{ // 7
	std::string("CDELT1"),
	std::string("coordinate increment along axis 1"),
	std::type_index(typeid(double)),
	true
},
{ // 8
	std::string("CDELT2"),
	std::string("coordinate increment along axis 2"),
	std::type_index(typeid(double)),
	true
},
{ // 9
	std::string("CDELT3"),
	std::string("coordinate increment along axis 3"),
	std::type_index(typeid(double)),
	true
},
{ // 10
	std::string("COMMENT"),
	std::string("descriptive comment"),
	std::type_index(typeid(void)),
	false
},
{ // 11
	std::string("CROTA1"),
	std::string("coordinate system rotation angle"),
	std::type_index(typeid(double)),
	true
},
{ // 12
	std::string("CROTA2"),
	std::string("coordinate system rotation angle"),
	std::type_index(typeid(double)),
	true
},
{ // 13
	std::string("CROTA3"),
	std::string("coordinate system rotation angle"),
	std::type_index(typeid(double)),
	true
},
{ // 14
	std::string("CRPIX1"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 15
	std::string("CRPIX2"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 16
	std::string("CRPIX3"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 17
	std::string("CRPIX2"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 18
	std::string("CRPIX3"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 19
	std::string("CRVAL1"),
	std::string("coordinate system value at reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 20
	std::string("CRVAL2"),
	std::string("coordinate system value at reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 21
	std::string("CRVAL3"),
	std::string("coordinate system value at reference pixel"),
	std::type_index(typeid(double)),
	true
},
{ // 22
	std::string("CTYPE1"),
	std::string("name of the coordinate axis 1"),
	std::type_index(typeid(std::string)),
	true
},
{ // 23
	std::string("CTYPE2"),
	std::string("name of the coordinate axis 1"),
	std::type_index(typeid(std::string)),
	true
},
{ // 24
	std::string("CTYPE3"),
	std::string("name of the coordinate axis 1"),
	std::type_index(typeid(std::string)),
	true
},
{ // 25
	std::string("DATAMIN"),
	std::string("minimum data value"),
	std::type_index(typeid(double)),
	true
},
{ // 26
	std::string("DATAMAX"),
	std::string("maximum data value"),
	std::type_index(typeid(double)),
	true
},
{ // 27
	std::string("DATE"),
	std::string("date of file creation"),
	std::type_index(typeid(FITSdate)),
	true
},
{ // 28
	std::string("DATE-OBS"),
	std::string("date of observation"),
	std::type_index(typeid(FITSdate)),
	true
},
{ // 29
	std::string("EQUINOX"),
	std::string("equinox of celestial coordinate system"),
	std::type_index(typeid(double)),
	true
},
{ // 30
	std::string("EXTEND"),
	std::string("may the FITS file contain extensions?"),
	std::type_index(typeid(bool)),
	true
},
{ // 31
	std::string("EXTLEVEL"),
	std::string("hierarchical level of the extension"),
	std::type_index(typeid(long)),
	false
},
{ // 32
	std::string("EXTNAME"),
	std::string("name of the extension"),
	std::type_index(typeid(long)),
	false
},
{ // 33
	std::string("EXTVER"),
	std::string("version of the extension"),
	std::type_index(typeid(long)),
	false
},
{ // 34
	std::string("GCOUNT"),
	std::string("group count"),
	std::type_index(typeid(long)),
	false
},
{ // 35
	std::string("GROUPS"),
	std::string("indicate random groups structure"),
	std::type_index(typeid(bool)),
	false
},
{ // 36
	std::string("HISTORY"),
	std::string("processing history of data"),
	std::type_index(typeid(void)),
	false
},
{ // 37
	std::string("INSTRUME"),
	std::string("name of instrument"),
	std::type_index(typeid(std::string)),
	true
},
{ // 38
	std::string("NAXIS"),
	std::string("number of axes"),
	std::type_index(typeid(long)),
	true
},
{ // 39
	std::string("NAXIS1"),
	std::string("size of axis 1"),
	std::type_index(typeid(long)),
	true
},
{ // 40
	std::string("NAXIS2"),
	std::string("size of axis 2"),
	std::type_index(typeid(long)),
	true
},
{ // 41
	std::string("NAXIS3"),
	std::string("size of axis 3"),
	std::type_index(typeid(long)),
	true
},
{ // 42
	std::string("OBJECT"),
	std::string("name of observed object"),
	std::type_index(typeid(std::string)),
	true
},
{ // 43
	std::string("OBSERVER"),
	std::string("observer who acquired the data"),
	std::type_index(typeid(std::string)),
	true
},
{ // 44
	std::string("ORIGIN"),
	std::string("organization responsible for the data"),
	std::type_index(typeid(std::string)),
	true
},
{ // 45
	std::string("PCOUNT"),
	std::string("parameter count"),
	std::type_index(typeid(long)),
	true
},
{ // 46
	std::string("PSCAL0"),
	std::string("parameter scaling factor"),
	std::type_index(typeid(double)),
	true
},
{ // 47
	std::string("PTYPE0"),
	std::string("name of random groups parameter"),
	std::type_index(typeid(std::string)),
	true
},
{ // 48
	std::string("PZERO0"),
	std::string("parameter scaling zero point"),
	std::type_index(typeid(double)),
	true
},
{ // 49
	std::string("REFERENC"),
	std::string("bibliographic reference"),
	std::type_index(typeid(std::string)),
	true
},
{ // 50
	std::string("SIMPLE"),
	std::string("does file conform to the Standard?"),
	std::type_index(typeid(bool)),
	true
},
{ // 51
	std::string("TELESCOP"),
	std::string("name of telescope"),
	std::type_index(typeid(std::string)),
	true
},
{ // 52
	std::string("XTENSION"),
	std::string("makes beginning of a new HDU"),
	std::type_index(typeid(std::string)),
	false
},
// nonstandard keywords
{ // 53
	std::string("DECCENTR"),
	std::string("declination of image center in degrees"),
	std::type_index(typeid(double)),
	true
},
{ // 54
	std::string("DECHIGHT"),
	std::string("height of image in declination degrees"),
	std::type_index(typeid(double)),
	true
},
{ // 55
	std::string("RACENTR"),
	std::string("right ascension of image center in hours"),
	std::type_index(typeid(double)),
	true
},
{ // 56
	std::string("RAWIDTH"),
	std::string("width of image in right ascension hours"),
	std::type_index(typeid(double)),
	true
},
{ // 57
	std::string("PXLWIDTH"),
	std::string("width of a pixeln in microns"),
	std::type_index(typeid(double)),
	true
},
{ // 58
	std::string("PXLHIGHT"),
	std::string("hight of pixel in microns"),
	std::type_index(typeid(double)),
	true
},
{ // 59
	std::string("FOCAL"),
	std::string("focal length of instrumen in meteres"),
	std::type_index(typeid(double)),
	true
},
{ // 60
	std::string("XOFFSET"),
	std::string("x offset of image center"),
	std::type_index(typeid(double)),
	true
},
{ // 61
	std::string("YOFFSET"),
	std::string("y offset of image center"),
	std::type_index(typeid(double)),
	true
},
{ // 62
	std::string("EXPTIME"),
	std::string("exposure time in seconds"),
	std::type_index(typeid(double)),
	true
},
{ // 63
	std::string("XBINNING"),
	std::string("binning in x direction"),
	std::type_index(typeid(long)),
	true
},
{ // 64
	std::string("YBINNING"),
	std::string("binning in y direction"),
	std::type_index(typeid(long)),
	true
},
{ // 65
	std::string("XORGSUBF"),
	std::string("x origin of subframe"),
	std::type_index(typeid(long)),
	true
},
{ // 66
	std::string("YORGSUBF"),
	std::string("y origin of subframe"),
	std::type_index(typeid(long)),
	true
},
{ // 67
	std::string("SET-TEMP"),
	std::string("set temperature of CCD in degrees C"),
	std::type_index(typeid(double)),
	true
},
{ // 68
	std::string("CCD-TEMP"),
	std::string("actual temperature of CCD in degrees C"),
	std::type_index(typeid(double)),
	true
},
{ // 69
	std::string("FILTER"),
	std::string("name of filter"),
	std::type_index(typeid(std::string)),
	true
},
{ // 70
	std::string("BAYER"),
	std::string("Bayer RGB filter layout"),
	std::type_index(typeid(std::string)),
	true
},
{ // 71
	std::string("IMAGEID"),
	std::string("Image id in repositry"),
	std::type_index(typeid(long)),
	true
},
{ // 72
	std::string("PURPOSE"),
	std::string("Purpose of an image: dark, flat or light"),
	std::type_index(typeid(std::string)),
	true
},
{ // 73
	std::string("PROJECT"),
	std::string("project this image was taken for"),
	std::type_index(typeid(std::string)),
	true
},
{ // 74
	std::string("UUID"),
	std::string("UUID of images"),
	std::type_index(typeid(std::string)),
	true
},
{ // 75
	std::string("TELALT"),
	std::string("Telescope altitude in degrees"),
	std::type_index(typeid(float)),
	true
},
{ // 76
	std::string("TELAZ"),
	std::string("Telescope azimut in degrees"),
	std::type_index(typeid(float)),
	true
},
{ // 77
	std::string("LATITUDE"),
	std::string("Observatory latitude in degrees"),
	std::type_index(typeid(float)),
	true
},
};

int	FITSKeywords::type(std::type_index idx) {
	if (idx == std::type_index(typeid(bool))) {
		return TLOGICAL;
	}
	if (idx == std::type_index(typeid(unsigned char))) {
		return TBYTE;
	}
	if (idx == std::type_index(typeid(char))) {
		return TSBYTE;
	}
	if (idx == std::type_index(typeid(std::string))) {
		return TSTRING;
	}
	if (idx == std::type_index(typeid(unsigned short))) {
		return TUSHORT;
	}
	if (idx == std::type_index(typeid(short))) {
		return TSHORT;
	}
	if (idx == std::type_index(typeid(unsigned int))) {
		return TUINT;
	}
	if (idx == std::type_index(typeid(int))) {
		return TINT;
	}
	if (idx == std::type_index(typeid(unsigned long))) {
		return TULONG;
	}
	if (idx == std::type_index(typeid(long))) {
		return TLONG;
	}
	if (idx == std::type_index(typeid(float))) {
		return TFLOAT;
	}
	if (idx == std::type_index(typeid(double))) {
		return TDOUBLE;
	}
	if (idx == std::type_index(typeid(long long))) {
		return TLONGLONG;
	}
	std::string	msg = stringprintf("type index '%s' not known",
				idx.name());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::invalid_argument(msg);
}

std::type_index	FITSKeywords::index(int tp) {
	switch (tp) {
	case TLOGICAL:	return std::type_index(typeid(bool));
	case TBIT:	return std::type_index(typeid(bool));
	case TBYTE:	return std::type_index(typeid(unsigned char));
	case TSBYTE:	return std::type_index(typeid(char));
	case TSTRING:	return std::type_index(typeid(std::string));
	case TINT:	return std::type_index(typeid(int));
	case TUINT:	return std::type_index(typeid(unsigned int));
	case TSHORT:	return std::type_index(typeid(short));
	case TUSHORT:	return std::type_index(typeid(unsigned short));
	case TLONG:	return std::type_index(typeid(long));
	case TULONG:	return std::type_index(typeid(unsigned long));
	case TLONGLONG:	return std::type_index(typeid(long long));
	case TFLOAT:	return std::type_index(typeid(float));
	case TDOUBLE:	return std::type_index(typeid(double));
	default:
		throw std::invalid_argument("unknown type");
	}
}

static std::set<std::string>	nameset;
static std::map<std::string, FITSKeyword>	keywordmap;

/**
 * \brief Build the name set and the map for the keyword data
 *
 * This function should not be called more than once, we use the
 * nameset_once object and the std::call_once method for this purpose
 */
static void	build_nameset() {
	for (int i = 0; i < Nkeywors; i++) {
		nameset.insert(keywors[i].name);
		keywordmap.insert(std::make_pair(keywors[i].name,
			keywors[i]));
	}
}

std::once_flag	nameset_once;

/**
 * \brief Get the set of vaild keyword names
 *
 * This method has to ensure that the name set is already initialized, it uses
 * the std::call_once call to build the set via the build_nameset method.
 */
const std::set<std::string>&	FITSKeywords::names() {
	std::call_once(nameset_once, build_nameset);
	return nameset;
}

/**
 * \brief find out whether a given name is known
 */
bool	FITSKeywords::known(const std::string& name) {
	return (names().find(name) != names().end());
}

/**
 * \brief Get the type of a keyword
 *
 * This is based ont the index method and the standard conversions from
 * types to FITS type codes
 */
int	FITSKeywords::type(const std::string& name) {
	return FITSKeywords::type(FITSKeywords::index(name));
}

/**
 * \brief find the keyword structure based on the name
 *
 * This static function has to make sure that the keyword structures,
 * the name set and the keyword map is already initialized. It does this
 * using the std::call_once method to the build_nameset function.
 */
static const FITSKeyword&	keyword(const std::string& name) {
	std::call_once(nameset_once, build_nameset);
	std::map<std::string, FITSKeyword>::const_iterator	k
		= keywordmap.find(name);
	if (keywordmap.find(name) == keywordmap.end()) {
		std::string	msg = stringprintf("unknown FITS keyword '%s'",
			name.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return k->second;
}

/**
 * \brief Get the type index associated with a keyword name
 */
std::type_index	FITSKeywords::index(const std::string& name) {
	return keyword(name).index;
}

/**
 * \brief Factory method to create metavalues with the right comments
 */
Metavalue	FITSKeywords::meta(const std::string& name, long value,
			const std::string& comment) {
	FITSKeyword	k = keyword(name);
	return Metavalue(name, k.index, stringprintf("%ld", value),
		(comment.size() == 0) ? k.comment : comment);
}

/**
 * \brief Factory method to create metavalues with the right comments
 */
Metavalue	FITSKeywords::meta(const std::string& name, double value,
			const std::string& comment) {
	FITSKeyword	k = keyword(name);
	return Metavalue(name, k.index, stringprintf("%f", value),
		(comment.size() == 0) ? k.comment : comment);
}

/**
 * \brief Factory method to create metavalues with the right comments
 */
Metavalue	FITSKeywords::meta(const std::string& name,
			const std::string& value, const std::string& comment) {
	FITSKeyword	k = keyword(name);
	if (k.index == std::type_index(typeid(void))) {
		return Metavalue(name, k.index, "", value);
	} else {
		return Metavalue(name, k.index, value,
			(comment.size() == 0) ? k.comment : comment);
	}
}

/**
 * \brief Factory method to create metavalues with the right comments
 */
Metavalue	FITSKeywords::meta(const std::string& name,
			const FITSdate& value, const std::string& comment) {
	FITSKeyword	k = keyword(name);
	return Metavalue(name, k.index, value.showVeryLong(),
			(comment.size() == 0) ? k.comment : comment);
}

/**
 * \brief Factory method to convert metavalues from FITShdu
 */
Metavalue	FITSKeywords::meta(const FITShdu& hdu) {
	std::string	key = hdu.name;
	std::string	value = hdu.value;
	std::string	comment = hdu.comment;
	return Metavalue(key, hdu.type, value, comment);
}

/**
 * \brief Get the standard comment for this keyword
 */
const std::string&	FITSKeywords::comment(const std::string& name) {
	return keyword(name).comment;
}

/**
 * \brief Find out whether the header can appear multiple times
 */
bool	FITSKeywords::unique(const std::string& name) {
	return keyword(name).unique;
}

} // namespace io
} // namespace stro
