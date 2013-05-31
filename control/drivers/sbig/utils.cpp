/*
 * utils.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <sbigudrv.h>
#include <string>
#include <utils.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief format an error message with info from the SBIG library
 *
 * The SBIG library offers a GET_ERROR_STRING method which allows
 * to retrieve information about an error. This method formats the
 * string and outputs it.
 */
std::string    sbig_error(short errorcode) {
	GetErrorStringParams    params;
	params.errorNo = errorcode;
	GetErrorStringResults   results;

	SBIGUnivDrvCommand(CC_GET_ERROR_STRING, &params, &results);
	return std::string(results.errorString);
}

SbigError::SbigError(short errorcode)
	: std::runtime_error(sbig_error(errorcode).c_str()) {
}

SbigError::SbigError(const char *cause) : std::runtime_error(cause) {
}

/**
 * \brief convert binning mode constants to binning mode objects
 */
Binning	SbigMode2Binning(unsigned short mode) {
	switch (mode & 0xff) {
	case RM_1X1:
		return Binning(1,1);
	case RM_2X2:
		return Binning(2,2);
	case RM_3X3:
		return Binning(3,3);
	case RM_9X9:
		return Binning(9,9);
	case RM_NX1:
		return Binning(-1,1);
	case RM_NX2:
		return Binning(-1,2);
	case RM_NX3:
		return Binning(-1,3);
	}
	throw std::logic_error("unknown binning mode");
}

unsigned short	SbigBinning2Mode(const Binning& mode) {
	if (mode == Binning(1,1)) {
		return RM_1X1;
	}
	if (mode == Binning(2,2)) {
		return RM_2X2;
	}
	if (mode == Binning(3,3)) {
		return RM_3X3;
	}
	if (mode == Binning(9,9)) {
		return RM_9X9;
	}
	if (mode.getY() == 1) {
		if (mode.getX() > 255) {
			throw std::range_error("X binning range too large");
		}
		return (mode.getX() << 8) || RM_NX1;
	}
	if (mode.getY() == 2) {
		if (mode.getX() > 255) {
			throw std::range_error("X binning range too large");
		}
		return (mode.getX() << 8) || RM_NX2;
	}
	if (mode.getY() == 3) {
		if (mode.getX() > 255) {
			throw std::range_error("X binning range too large");
		}
		return (mode.getX() << 8) || RM_NX3;
	}
	throw std::range_error("unknown binning mode");
}

} // namespace sbig
} // namespace camera
} // namespace astro
