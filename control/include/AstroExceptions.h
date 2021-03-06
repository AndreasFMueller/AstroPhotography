/*
 * AstroExceptions.h -- Exceptions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroExceptions_h
#define _AstroExceptions_h

#include <errno.h>
#include <stdexcept>

namespace astro {

/**
 * \brief Standard exception to report unix errors
 *
 * This exception should be used to report unix exceptions. It derives
 * from std::runtime_error, but it includes a simple facility to report
 * the unix error that happened.
 */
class runtime_errno : public std::runtime_error {
	int	_unix_errno;
public:
	runtime_errno(const char *cause, int unix_errno);
	const int&	unix_errno() const { return _unix_errno; }
};

/**
 * \brief Exception thrown when some object is not found
 */
class NotFound : public std::runtime_error {
public:
	NotFound(const char *cause) : std::runtime_error(cause) { }
	NotFound(const std::string& cause) : std::runtime_error(cause) { }
};

/**
 * \brief Exception thrown when some function is not implemented
 *
 * This exception is thrown by all the virtual functions of the device
 * interface classes. To prevent this, a concrete device driver class
 * must override the methods.
 */
class NotImplemented : public std::runtime_error {
public:
	NotImplemented(const char *cause) : std::runtime_error(cause) { }
	NotImplemented(const std::string& cause) : std::runtime_error(cause) { }
};

/**
 * \brief Exception thrown when bad parameters are used with a call
 *
 * This exception is thrown e.g. when the exposure of a rectangle is
 * requested, that is larger than the CCD used. Or when a binning mode
 * is requested that is not supported by the camera.
 */
class BadParameter : public std::runtime_error {
public:
	BadParameter(const char *cause) : std::runtime_error(cause) { }
	BadParameter(const std::string& cause) : std::runtime_error(cause) { }
};

/**
 * \brief Parent exception class for device exceptions
 */
class DeviceException : public std::runtime_error {
public:
	DeviceException(const char *cause) : std::runtime_error(cause) { }
	DeviceException(const std::string& cause) : std::runtime_error(cause) { }
};

/**
 * \brief Exception thrown when a device times out
 */
class DeviceTimeout : public DeviceException {
public:
	DeviceTimeout(const char *cause) : DeviceException(cause) { }
	DeviceTimeout(const std::string& cause) : DeviceException(cause) { }
};

/**
 * \brief Exception thrown when a protocol error with a device is encountered
 */
class DeviceProtocolException : public DeviceException {
public:
	DeviceProtocolException(const char *cause)
		: DeviceException(cause) { }
	DeviceProtocolException(const std::string& cause)
		: DeviceException(cause) { }
};

namespace camera {

/**
 * \brief Exception thrown when the device is not in the correct state
 *
 * This exception is thrown e.g. when an image is requested although no
 * exposure was started, or if the image of the previous exposure has already
 * been retrieved.
 */
class BadState : public DeviceException {
public:
	BadState(const char *cause) : DeviceException(cause) { }
	BadState(const std::string& cause) : DeviceException(cause) { }
};
	
} // namespace camera

} // namespace astro

#endif /* _AstroExceptions.h */
