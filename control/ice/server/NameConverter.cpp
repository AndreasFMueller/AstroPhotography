/*
 * NameConverter.cpp -- convert names into samething acceptable to ICE
 *
 * (c) 2014 Prof Dr Andreas Mueller
 */
#include <NameConverter.h>
#include <sstream>
#include <iomanip>
#include <AstroDebug.h>

namespace snowstar {

std::string	NameConverter::urlencode(const std::string& name) {
	std::ostringstream	out;
	for (int i = 0; i < name.size(); i++) {
		char	c = name[i];
		if ((('a' <= c) && (c <= 'z')) ||
			(('A' <= c) && (c <= 'Z')) ||
			(('0' <= c) && (c <= '9'))) {
			out << c;
		} else {
			out << "%";
			out << std::setw(2) << std::hex << (int)c;
		}
	}
	std::string	s = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s", name.c_str(), s.c_str());
	return s;
}

std::string	NameConverter::urldecode(const std::string& name) {
	std::ostringstream	out;
	int i = 0;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s", name.c_str(), s.c_str());
	return s;
}

} // namespace snowstar
