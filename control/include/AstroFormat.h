//
// AstroFormat.h -- format numbers as std::string
//
#ifndef _AstroFormat_h
#define _AstroFormat_h

#include <string>
#include <cstdarg>

namespace astro {

extern std::string stringprintf(const char *format, ...);
extern std::string stringprintf(const std::string format, ...);
extern std::string vstringprintf(const char *format, va_list args);
extern std::string vstringprintf(const std::string format, va_list args);

} // namespace rocket

#endif	/* _AstroFormat_h */
