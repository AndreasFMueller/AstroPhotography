/*
 * URL.cpp -- URL implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <stdexcept>

namespace astro {

/**
 * \brief construct an URL from an URL string
 */
URL::URL(const std::string& urlstring) {
	// separate method from rest
	std::string::size_type	pos = urlstring.find(':');
	if (std::string::npos == pos) {
		throw std::invalid_argument("method missing");
	}
	_method = urlstring.substr(0, pos);
	std::string	rest = urlstring.substr(pos + 1);

	// split the rest into path components
	std::vector<std::string>	parts;
	split<std::vector<std::string> >(rest, "/", parts);
	std::copy(parts.begin(), parts.end(), back_inserter(*this));
}

/**
 * \brief convert an URL to string
 */
URL::operator std::string() const {
	return _method + ":" + Concatenator::concat(*this, "/");
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
