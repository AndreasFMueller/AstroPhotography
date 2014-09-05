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
} FITSKeyword;

#define	Nkeywors	72
FITSKeyword	keywors[Nkeywors] = {
// standard keywords
{ // 0
	std::string("AUTHOR"),
	std::string("author of the data"),
	std::type_index(typeid(std::string))
},
{ // 1
	std::string("BITPIX"),
	std::string("bits per data value"),
	std::type_index(typeid(long))
},
{ // 2
	std::string("BLANK"),
	std::string("value used for undefined array elements"),
	std::type_index(typeid(long))
},
{ // 3
	std::string("BLOCKED"),
	std::string("is physical blocksize a multiple of 2880"),
	std::type_index(typeid(bool))
},
{ // 4
	std::string("BSCALE"),
	std::string("linear factor in scaling equation"),
	std::type_index(typeid(double))
},
{ // 5
	std::string("BUNIT"),
	std::string("physical units of the array values"),
	std::type_index(typeid(std::string))
},
{ // 6
	std::string("BZERO"),
	std::string("zero point in scaling equation"),
	std::type_index(typeid(double))
},
{ // 7
	std::string("CDELT1"),
	std::string("coordinate increment along axis 1"),
	std::type_index(typeid(double))
},
{ // 8
	std::string("CDELT2"),
	std::string("coordinate increment along axis 2"),
	std::type_index(typeid(double))
},
{ // 9
	std::string("CDELT3"),
	std::string("coordinate increment along axis 3"),
	std::type_index(typeid(double))
},
{	// special type just as history, 10
	std::string("COMMENT"),
	std::string("descriptive comment"),
	std::type_index(typeid(std::string)),
},
{ // 11
	std::string("CROTA1"),
	std::string("coordinate system rotation angle"),
	std::type_index(typeid(double))
},
{ // 12
	std::string("CROTA2"),
	std::string("coordinate system rotation angle"),
	std::type_index(typeid(double))
},
{ // 13
	std::string("CROTA3"),
	std::string("coordinate system rotation angle"),
	std::type_index(typeid(double))
},
{ // 14
	std::string("CRPIX1"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double))
},
{ // 15
	std::string("CRPIX2"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double))
},
{ // 16
	std::string("CRPIX3"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double))
},
{ // 17
	std::string("CRPIX2"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double))
},
{ // 18
	std::string("CRPIX3"),
	std::string("coordinate system reference pixel"),
	std::type_index(typeid(double))
},
{ // 19
	std::string("CRVAL1"),
	std::string("coordinate system value at reference pixel"),
	std::type_index(typeid(double))
},
{ // 20
	std::string("CRVAL2"),
	std::string("coordinate system value at reference pixel"),
	std::type_index(typeid(double))
},
{ // 21
	std::string("CRVAL3"),
	std::string("coordinate system value at reference pixel"),
	std::type_index(typeid(double))
},
{ // 22
	std::string("CTYPE1"),
	std::string("name of the coordinate axis 1"),
	std::type_index(typeid(std::string))
},
{ // 23
	std::string("CTYPE2"),
	std::string("name of the coordinate axis 1"),
	std::type_index(typeid(std::string))
},
{ // 24
	std::string("CTYPE3"),
	std::string("name of the coordinate axis 1"),
	std::type_index(typeid(std::string))
},
{ // 25
	std::string("DATAMIN"),
	std::string("minimum data value"),
	std::type_index(typeid(double))
},
{ // 26
	std::string("DATAMAX"),
	std::string("maximum data value"),
	std::type_index(typeid(double))
},
{ // 27
	std::string("DATE"),
	std::string("date of file creation"),
	std::type_index(typeid(FITSdate))
},
{ // 28
	std::string("DATE-OBS"),
	std::string("date of observation"),
	std::type_index(typeid(FITSdate))
},
{ // 29
	std::string("EQUINOX"),
	std::string("equinox of celestial coordinate system"),
	std::type_index(typeid(double))
},
{ // 30
	std::string("EXTEND"),
	std::string("may the FITS file contain extensions?"),
	std::type_index(typeid(bool))
},
{ // 31
	std::string("EXTLEVEL"),
	std::string("hierarchical level of the extension"),
	std::type_index(typeid(long))
},
{ // 32
	std::string("EXTNAME"),
	std::string("name of the extension"),
	std::type_index(typeid(long))
},
{ // 33
	std::string("EXTVER"),
	std::string("version of the extension"),
	std::type_index(typeid(long))
},
{ // 34
	std::string("GCOUNT"),
	std::string("group count"),
	std::type_index(typeid(long))
},
{ // 35
	std::string("GROUPS"),
	std::string("indicate random groups structure"),
	std::type_index(typeid(bool))
},
{	// we need a special type, these records have no command field, 36
	std::string("HISTORY"),
	std::string("processing history of data"),
	std::type_index(typeid(std::string))
},
{ // 37
	std::string("INSTRUME"),
	std::string("name of instrument"),
	std::type_index(typeid(std::string))
},
{ // 38
	std::string("NAXIS"),
	std::string("number of axes"),
	std::type_index(typeid(long))
},
{ // 39
	std::string("NAXIS1"),
	std::string("size of axis 1"),
	std::type_index(typeid(long))
},
{ // 40
	std::string("NAXIS2"),
	std::string("size of axis 2"),
	std::type_index(typeid(long))
},
{ // 41
	std::string("NAXIS3"),
	std::string("size of axis 3"),
	std::type_index(typeid(long))
},
{ // 42
	std::string("OBJECT"),
	std::string("name of observed object"),
	std::type_index(typeid(std::string))
},
{ // 43
	std::string("OBSERVER"),
	std::string("observer who acquired the data"),
	std::type_index(typeid(std::string))
},
{ // 44
	std::string("ORIGIN"),
	std::string("organization responsible for the data"),
	std::type_index(typeid(std::string))
},
{ // 45
	std::string("PCOUNT"),
	std::string("parameter count"),
	std::type_index(typeid(long))
},
{ // 46
	std::string("PSCAL0"),
	std::string("parameter scaling factor"),
	std::type_index(typeid(double))
},
{ // 47
	std::string("PTYPE0"),
	std::string("name of random groups parameter"),
	std::type_index(typeid(std::string))
},
{ // 48
	std::string("PZERO0"),
	std::string("parameter scaling zero point"),
	std::type_index(typeid(double))
},
{ // 49
	std::string("REFERENC"),
	std::string("bibliographic reference"),
	std::type_index(typeid(std::string))
},
{ // 50
	std::string("SIMPLE"),
	std::string("does file conform to the Standard?"),
	std::type_index(typeid(bool))
},
{ // 51
	std::string("TELESCOP"),
	std::string("name of telescope"),
	std::type_index(typeid(std::string))
},
{ // 52
	std::string("XTENSION"),
	std::string("makes beginning of a new HDU"),
	std::type_index(typeid(std::string))
},
// nonstandard keywords
{ // 53
	std::string("DECCENTR"),
	std::string("declination of image center in degrees"),
	std::type_index(typeid(double))
},
{ // 54
	std::string("DECHIGHT"),
	std::string("height of image in declination degrees"),
	std::type_index(typeid(double))
},
{ // 55
	std::string("RACENTR"),
	std::string("right ascension of image center in hours"),
	std::type_index(typeid(double))
},
{ // 56
	std::string("RAWIDTH"),
	std::string("width of image in right ascension hours"),
	std::type_index(typeid(double))
},
{ // 57
	std::string("PXLWIDTH"),
	std::string("width of a pixeln in microns"),
	std::type_index(typeid(double))
},
{ // 58
	std::string("PXLHIGHT"),
	std::string("hight of pixel in microns"),
	std::type_index(typeid(double))
},
{ // 59
	std::string("FOCAL"),
	std::string("focal length of instrumen in meteres"),
	std::type_index(typeid(double))
},
{ // 60
	std::string("XOFFSET"),
	std::string("x offset of image center"),
	std::type_index(typeid(double))
},
{ // 61
	std::string("YOFFSET"),
	std::string("y offset of image center"),
	std::type_index(typeid(double))
},
{ // 62
	std::string("EXPTIME"),
	std::string("exposure time in seconds"),
	std::type_index(typeid(double))
},
{ // 63
	std::string("XBINNING"),
	std::string("binning in x direction"),
	std::type_index(typeid(long))
},
{ // 64
	std::string("YBINNING"),
	std::string("binning in y direction"),
	std::type_index(typeid(long))
},
{ // 65
	std::string("XORGSUBF"),
	std::string("x origin of subframe"),
	std::type_index(typeid(long))
},
{ // 66
	std::string("YORGSUBF"),
	std::string("y origin of subframe"),
	std::type_index(typeid(long))
},
{ // 67
	std::string("SET-TEMP"),
	std::string("set temperature of CCD in degrees C"),
	std::type_index(typeid(double))
},
{ // 68
	std::string("CCD-TEMP"),
	std::string("actual temperature of CCD in degrees C"),
	std::type_index(typeid(double))
},
{ // 69
	std::string("FILTER"),
	std::string("name of filter"),
	std::type_index(typeid(std::string))
},
{ // 70
	std::string("BAYER"),
	std::string("Bayer RGB filter layout"),
	std::type_index(typeid(std::string))
},
{ // 71
	std::string("IMAGEID"),
	std::string("Image id in repositry"),
	std::type_index(typeid(long))
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
		std::string	msg = stringprintf("unknow FITS keyword '%s'",
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
Metavalue	FITSKeywords::meta(const std::string& name, long value) {
	FITSKeyword	k = keyword(name);
	return Metavalue(name, k.index, stringprintf("%ld", value), k.comment);
}

/**
 * \brief Factory method to create metavalues with the right comments
 */
Metavalue	FITSKeywords::meta(const std::string& name, double value) {
	FITSKeyword	k = keyword(name);
	return Metavalue(name, k.index, stringprintf("%f", value), k.comment);
}

/**
 * \brief Factory method to create metavalues with the right comments
 */
Metavalue	FITSKeywords::meta(const std::string& name,
			const std::string& value) {
	FITSKeyword	k = keyword(name);
	return Metavalue(name, k.index, value, k.comment);
}

/**
 * \brief Factory method to create metavalues with the right comments
 */
Metavalue	FITSKeywords::meta(const std::string& name,
			const FITSdate& value) {
	FITSKeyword	k = keyword(name);
	return Metavalue(name, k.index, value.showLong(), k.comment);
}

/**
 * \brief Get the standard comment for this keyword
 */
const std::string&	FITSKeywords::comment(const std::string& name) {
	return keyword(name).comment;
}

} // namespace io
} // namespace stro
