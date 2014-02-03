/*
 * NameConverter.h -- convert names to a version acceptable to ICE
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NameConverter_h
#define _NameConverter_h

#include <string>

namespace snowstar {

class NameConverter {
public:
static std::string	urlencode(const std::string& name);
static std::string	urldecode(const std::string& name);
};

} // namespace snowstar

#endif /* _NameConverter_h */
