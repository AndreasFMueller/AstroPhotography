/*
 * utils.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <sbigudrv.h>
#include <string>
#include <utils.h>

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

} // namespace sbig
} // namespace camera
} // namespace astro
