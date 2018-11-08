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
#include <netdb.h>
#include <arpa/inet.h>
#include <includes.h>
#include <sstream>

namespace astro {

/**
 * \brief construct an URL from an URL string
 */
URL::URL(const std::string& urlstring) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing '%s'", urlstring.c_str());
	// use a regular expression to find the parts of the URL
	//                 1        2  3          4 5           6               8
	//                 method      hostname
	std::string	r("([a-z]*):(//([a-zA-Z0-9\\.]+)(:([0-9]+))?)?(/?([0-9a-zA-Z\\.]*)(/[-0-9a-zA-Z\\.]+)*)");
	std::regex	rx(r, std::regex::extended);
	std::smatch	matchresults;

	if (!regex_match(urlstring, matchresults, rx)) {
		std::string	msg = stringprintf("url '%s' does not match "
			"regex '%s'", urlstring.c_str(), r.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// method
	_method = matchresults[1];
	if (_method == "http") {
		port(80);
	}

	// server and port
	if (matchresults.length(3) > 0) {
		host(matchresults[3]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "server: '%s'", host().c_str());
	}
	if (matchresults.length(5) > 0) {
		std::string	portstring = matchresults[5];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "portstring: '%s' %u",
			portstring.c_str(), matchresults.position(5));
		port(std::stoi(matchresults[5]));
	}

	// separate method from rest
	std::string	rest = matchresults[6];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rest = '%s'", rest.c_str());

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
		case '!':
			result.append("%21");
			break;
		case '#':
			result.append("%23");
			break;
		case '$':
			result.append("%24");
			break;
		case '%':
			result.append("%25");
			break;
		case '&':
			result.append("%26");
			break;
		case '\'':
			result.append("%27");
			break;
		case '(':
			result.append("%28");
			break;
		case ')':
			result.append("%29");
			break;
		case '*':
			result.append("%2A");
			break;
		case '+':
			result.append("%2B");
			break;
		case ',':
			result.append("%2C");
			break;
		case '/':
			result.append("%2F");
			break;
		case ':':
			result.append("%3A");
			break;
		case '=':
			result.append("%3D");
			break;
		case ';':
			result.append("%3B");
			break;
		case '?':
			result.append("%3F");
			break;
		case '@':
			result.append("%40");
			break;
		case '[':
			result.append("%5B");
			break;
		case ']':
			result.append("%5D");
			break;
		case ' ':
			result.append("+");
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
		} if ('+' == c) {
			result.append(" ");
		} else {
			result.append(1, c);
			pos++;
		}
	}
	return result;
}

/**
 * \brief 
 */
int	URL::post(const PostData& data) {
	// resolve the host name
	struct sockaddr_in	sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote port: %hu", port());
	struct hostent	*hep = gethostbyname(host().c_str());
	if (!hep) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot resolve '%s': %s",
			host().c_str(), strerror(errno));
		return -1;
	}
	memcpy(&sa.sin_addr, hep->h_addr, 4);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote IP: %s", inet_ntoa(sa.sin_addr));

	// create a connection
	int	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot create socket: %s",
			strerror(errno));
		return -1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote connection created");

	// connect to the remote server
	if (::connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot connect: %s",
			strerror(errno));
		close(fd);
		return -1;
	}

	// prepare to write 
	std::string	d = data.urlEncode();

	// write the command
	std::ostringstream	out;
	out << "POST " << path() << " HTTP/1.0" << "\r\n";

	// write the headers
	out << "Host: " << host() << "\r\n";
	out << "Content-Type: application/x-www-form-urlencoded" << "\r\n";
	out << "Content-Length: " << d.size() << "\r\n";
	out << "\r\n";
	out << d;
	out << "\r\n";
	debug(LOG_DEBUG, DEBUG_LOG, 0, "posting: %s", out.str().c_str());

	write(fd, out.str().data(), out.str().size());

	// read the response
	char	buffer[10000];
	memset(buffer, 0, sizeof(buffer));
	ssize_t	bytes = read(fd, buffer, sizeof(buffer));
	std::string	response(buffer, bytes);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s", response.c_str());
	close(fd);
	if (bytes < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get response: %s",
			strerror(errno));
		return -1;
	}
	if (bytes < 20) {
		debug(LOG_ERR, DEBUG_LOG, 0, "did not get response large enough");
		return -1;
	}
	char	*c = strchr(buffer, ' ');
	if (c == NULL) {
		return -1;
	}
	while (*c == ' ') { c++; }
	int	rc = atoi(c);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "response code: %d", rc);
	return rc;
}

} // namespace astro
