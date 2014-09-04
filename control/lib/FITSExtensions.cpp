/*
 * FITSExtensions.cpp -- implementation of type mapping functions for attributes
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
	std::type_index	index;
} FITSExtension;

#define	Nextensions	31
FITSExtension	extensions[Nextensions] = {
	{ std::string("SIMPLE"),	std::type_index(typeid(bool))	}, //  0
	{ std::string("EXTEND"),	std::type_index(typeid(bool))	}, //  1
	{ std::string("NAXIS"),		std::type_index(typeid(long))	}, //  2
	{ std::string("NAXIS1"),	std::type_index(typeid(long))	}, //  3
	{ std::string("NAXIS2"),	std::type_index(typeid(long))	}, //  4
	{ std::string("NAXIS3"),	std::type_index(typeid(long))	}, //  5
	{ std::string("BZERO"),		std::type_index(typeid(double))	}, //  6
	{ std::string("BSCALE"),	std::type_index(typeid(double))	}, //  7
	{ std::string("DATAMIN"),	std::type_index(typeid(double))	}, //  8
	{ std::string("DATAMAX"),	std::type_index(typeid(double))	}, //  9
	{ std::string("DECCENTR"),	std::type_index(typeid(double))	}, // 10
	{ std::string("DECHIGHT"),	std::type_index(typeid(double))	}, // 11
	{ std::string("DECWIDTH"),	std::type_index(typeid(double))	}, // 12
	{ std::string("RACENTR"),	std::type_index(typeid(double))	}, // 13
	{ std::string("RAHIGHT"),	std::type_index(typeid(double))	}, // 14
	{ std::string("RAWIDTH"),	std::type_index(typeid(double))	}, // 15
	{ std::string("PXLWIDTH"),	std::type_index(typeid(double))	}, // 16
	{ std::string("PXLHIGHT"),	std::type_index(typeid(double))	}, // 17
	{ std::string("FOCAL"),		std::type_index(typeid(double))	}, // 18
	{ std::string("XOFFSET"),	std::type_index(typeid(double))	}, // 19
	{ std::string("YOFFSET"),	std::type_index(typeid(double))	}, // 20
	{ std::string("EXPTIME"),	std::type_index(typeid(double))	}, // 21
	{ std::string("XBINNING"),	std::type_index(typeid(long))	}, // 22
	{ std::string("YBINNING"),	std::type_index(typeid(long))	}, // 23
	{ std::string("XORGSUBF"),	std::type_index(typeid(long))	}, // 24
	{ std::string("YORGSUBF"),	std::type_index(typeid(long))	}, // 25
	{ std::string("HISTORY"),std::type_index(typeid(std::string))	}, // 26
	{ std::string("COMMENT"),std::type_index(typeid(std::string))	}, // 27
	{ std::string("SET-TEMP"),	std::type_index(typeid(double))	}, // 28
	{ std::string("CCD-TEMP"),	std::type_index(typeid(double))	}, // 29
	{ std::string("FILTER"), std::type_index(typeid(std::string))	}, // 30
};

int	FITSExtensions::type(const std::string& name) {
	return FITSExtensions::type(FITSExtensions::index(name));
}

std::type_index	FITSExtensions::index(const std::string& name) {
	for (int i = 0; i < Nextensions; i++) {
		if (extensions[i].name == name) {
			return extensions[i].index;
		}
	}
	std::string	msg = stringprintf("extension name '%s' not known",
				name.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::invalid_argument(msg);
}

int	FITSExtensions::type(std::type_index idx) {
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

std::type_index	FITSExtensions::index(int tp) {
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

void	build_nameset() {
	for (int i = 0; i < Nextensions; i++) {
		nameset.insert(extensions[i].name);
	}
}

std::once_flag	nameset_once;

const std::set<std::string>&	FITSExtensions::names() {
	std::call_once(nameset_once, build_nameset);
	return nameset;
}

} // namespace io
} // namespace stro
