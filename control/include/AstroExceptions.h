/*
 * AstroExceptions.h -- Exceptions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroExceptions_h

#include <stdexcept>

namespace astro {

class NotFound : public std::runtime_error {
public:
	NotFound(const char *cause) : std::runtime_error(cause) { }
};

class NotImplemented : public std::runtime_error {
public:
	NotImplemented(const char *cause) : std::runtime_error(cause) { }
};

class BadParameter : public std::runtime_error {
public:
	BadParameter(const char *cause) : std::runtime_error(cause) { }
};

namespace camera {

class BadState : public std::runtime_error {
public:
	BadState(const char *cause) : std::runtime_error(cause) { }
};
	
} // namespace camera

} // namespace astro

#endif /* _AstroExceptions.h */
