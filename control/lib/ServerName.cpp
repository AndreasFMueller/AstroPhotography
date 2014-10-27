/*
 * ServerName.cpp -- server name parsing class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <includes.h>
#include <AstroDebug.h>

namespace astro {

const unsigned short	default_port = 10000;

static unsigned short	icestar_port() {
	// check whether there is an entry in services
	struct servent	*serv = getservbyname("snowstar", "tcp");
	if (NULL == serv) {
		return default_port;
	}
	return serv->s_port;
}

ServerName::ServerName() : _host("localhost"), _port(icestar_port()) {
}

ServerName::ServerName(const std::string& host, unsigned short port)
	: _host(host), _port(port) {
}

ServerName::ServerName(const std::string& servername) {
	std::string::size_type	pos = servername.find(':');
	if (pos == std::string::npos) {
		_host = servername;
		_port = icestar_port();
		return;
	}
	_host = servername.substr(0, pos);
	_port = std::stoi(servername.substr(pos + 1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "host = %s, port = %hu", _host.c_str(),
		_port);
}

ServerName::operator	std::string() const {
	return stringprintf("%s:%hu", _host.c_str(), _port);
}

std::string	ServerName::connect(const std::string& service) const {
	std::string	connectstring = stringprintf("%s:default -h %s -p %hu",
		service.c_str(), host().c_str(), port());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connecting to %s",
		connectstring.c_str());
	return connectstring;
}

bool	ServerName::isDefault() const {
	return isDefaultPort() && (_host == std::string("localhost"));
}

bool	ServerName::isDefaultPort() const {
	return _port == 10000;
}

} // namespace astro
