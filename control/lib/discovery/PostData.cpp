/*
 * PostData.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <sstream>

namespace astro {

PostData::PostData() {
}

PostData::~PostData() {
}

std::string	PostData::urlEncode() const {
	std::ostringstream	out;
	bool	first = true;
	std::for_each(begin(), end(),
		[&](const std::pair<std::string, std::string>& p) {
			if (!first) out << "&";
			first = false;
			out << p.first << "=" << URL::encode(p.second);
		}
	);
	return out.str();
}

} // namespace astro
