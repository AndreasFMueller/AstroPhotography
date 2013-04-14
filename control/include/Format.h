//
// Format.h -- format numbers as std::string
//
#ifndef _Format_h
#define _Format_h

#include <string>
#include <stdarg.h>

namespace astro {

extern std::string stringprintf(const char *format, ...);
extern std::string vstringprintf(const char *format, va_list args);

} // namespace rocket

#endif	/* _Format_h */
