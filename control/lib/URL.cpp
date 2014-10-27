/*
 * URL.cpp -- URL implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <regex>

namespace astro {

/**
 * \brief construct an URL from an URL string
 */
URL::URL(const std::string& urlstring) {
	// use a regular expression to find the parts of the URL
	//                 1        2  3          4 5           6               8
	//                 method      hostname
	std::string	r("([a-z]*):(//([a-z\\.]+)(:([0-9]+))?/)?(([0-9a-zA-Z]*)(/[-0-9a-zA-Z]+)*)");
	std::regex	rx(r, std::regex::extended);
	std::smatch	matchresults;

	if (!std::regex_match(urlstring, matchresults, rx)) {
		std::string	msg = stringprintf("url '%s' does not match "
			"regex '%s'", urlstring.c_str(), r.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

#if 0
	// the string matched, so we should be able to get the match results
	// from the expression
	int	i;
	std::smatch::iterator	ptr;
	for (i = 0, ptr = matchresults.begin(); ptr != matchresults.end();
		ptr++, i++) {
		std::cout << "[" << i << "] ";
		if (matchresults.position(i) >= 0) {
			std::cout << *ptr
				<< ", position=" << matchresults.position(i)
				<< " length=" << matchresults.position(i);
		}
		std::cout << std::endl;
	}
#endif

	// method
	_method = matchresults[1];

	// server and port
	if (matchresults.position(3) > 0) {
		host(matchresults[3]);
	}
	if (matchresults.position(5) > 0) {
		port(std::stoi(matchresults[5]));
	}

	// separate method from rest
	std::string	rest = matchresults[6];

	// split the rest into path components
	std::vector<std::string>	parts;
	split<std::vector<std::string> >(rest, "/", parts);
	std::copy(parts.begin(), parts.end(), back_inserter(*this));
}

/**
 * \brief Get the path part of the URL
 */
std::string	URL::path() const {
	return Concatenator::concat(*this, "/");
}

/**
 * \brief convert an URL to string
 */
URL::operator std::string() const {
	std::string	result = _method + ":";
	if (!isDefault()) {
		result += std::string("//") + (std::string)(ServerName)(*this)
			+ std::string("/");
	}
	result += path();
	return result;
}

/**
 * \brief Encode an URL
 *
 * URL metacharacters ('/', ':' and '%') are encoded by %xx, where xx 
 * represents the hex code of the character. This function replaces all
 * occurences of these characters by the %xx equivalent.
 */
std::string	URL::encode(const std::string& in) {
	std::string	result;
	std::string::size_type	pos = 0;
	while (pos < in.size()) {
		char	c = in[pos++];
		switch (c) {
		case '/':
			result.append("%2F");
			break;
		case ':':
			result.append("%3A");
			break;
		case '%':
			result.append("%25");
			break;
		default:
			// STL only allows to append multiple characters
			result.append(1, c);
			break;
		}
	}
	return result;
}

/**
 * \brief URL decode a string
 *
 * This function reverses the encoding implemented by the URL::encode
 * function.
 */
std::string	URL::decode(const std::string& in) {
	std::string	result;
	std::string::size_type	pos = 0;
	while (pos < in.size()) {
		char	c = in[pos];
		if ('%' == c) {
			int	l = std::stoi(in.substr(++pos, 2), NULL, 16);
			pos += 2;
			switch (l) {
			case 0x2f:
			case 0x3a:
			case 0x25:
				result.append(1, (char)l);
				break;
			default:
				debug(LOG_ERR, DEBUG_LOG, 0,
					"unknown escaped character: %x", l);
				throw std::invalid_argument("escaping error");
			}
		} else {
			result.append(1, c);
			pos++;
		}
	}
	return result;
}

} // namespace astro
