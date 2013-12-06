/*
 * SxUtils.h -- utilities for 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxUtils_h
#define _SxUtils_h

#include <stdexcept>
#include <AstroUSB.h>

namespace astro {
namespace camera {
namespace sx {


class SxError : public std::runtime_error {
public:
	SxError(const char *cause);
};

#define	SX_VENDOR_ID	0x1278

/**
 * \brief Auxiliary function to generate the camera name from the deviceptr
 */
extern std::string	sxname(astro::usb::DevicePtr& deviceptr);
extern void	sxparse(const std::string& name, int& busnumber,
			int& deviceaddress);

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxUtils_h */
