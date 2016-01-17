/*
 * NameConverter.cpp -- convert names into samething acceptable to ICE
 *
 * (c) 2014 Prof Dr Andreas Mueller
 */
#include <NameConverter.h>
#include <sstream>
#include <iomanip>
#include <AstroDebug.h>
#include <cctype>

namespace snowstar {

/**
 * \brief URLencode a string
 *
 * This is a rather primitive URL encoder: it converts everything that is
 * not alphanumeric (based on the isalnum function from cctype).
 */
std::string	NameConverter::urlencode(const std::string& name) {
	std::ostringstream	out;
	for (unsigned int i = 0; i < name.size(); i++) {
		char	c = name[i];
		if (isalnum(c)) {
			out << c;
		} else {
			out << "%";
			out << std::setw(2) << std::hex << (int)c;
		}
	}
	std::string	s = out.str();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s", name.c_str(), s.c_str());
	return s;
}

/**
 * \brief URLdecode a string
 *
 * URL decode strings. This isn't very intelligent, e.g. it does not check
 * whether the characters following the % sign really form a hex number.
 */
std::string	NameConverter::urldecode(const std::string& name) {
	std::ostringstream	out;
	unsigned int i = 0;
	while (i < name.size()) {
		if (name[i] == '%') {
			i++;
			char c = std::stoi(name.substr(i, 2), 0, 16);
			out << c;
			i += 2;
		} else {
			out << name[i];
			i++;
		}
	}
	std::string	s = out.str();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s", name.c_str(), s.c_str());
	return s;
}

} // namespace snowstar
