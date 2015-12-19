/*
 * ServerName.cpp -- server name parsing class
 *
 * Note: this class is located in the top level name space, but it uses
 *       the dynamic service discovery interface, so it must for the moment
 *       stay in the top level library
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroDiscovery.h>

using namespace astro::discover;

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

static ServiceObject	resolve(const std::string& name) {
	ServiceDiscoveryPtr	discovery = ServiceDiscovery::get();
	discovery->start();
	ServiceKey	key = discovery->waitfor(name);
	return discovery->find(key);
}

unsigned short	ServerName::port() const {
	if (!_isdynamic) {
		return _port;
	}
	return resolve(_host).port();
}

std::string	ServerName::host() const {
	if (!_isdynamic) {
		return _host;
	}
	return resolve(_host).host();
}

ServerName::ServerName() : _host("localhost"), _port(icestar_port()) {
	_isdynamic = false;
}

ServerName::ServerName(const std::string& host, unsigned short port)
	: _host(host), _port(port) {
	_isdynamic = false;
}

ServerName::ServerName(const std::string& servicename) {
	std::string::size_type	pos = servicename.find(':');
	if (pos == std::string::npos) {
		_isdynamic = true;
		_host = servicename;
		_port = icestar_port();
		return;
	}
	_host = servicename.substr(0, pos);
	_port = std::stoi(servicename.substr(pos + 1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "host = %s, port = %hu", _host.c_str(),
		_port);
}

ServerName::operator	std::string() const {
	return stringprintf("%s:%hu", _host.c_str(), _port);
}

std::string	ServerName::connect(const std::string& service) const {
	std::string	h;
	unsigned short	p;
	if (_isdynamic) {
		ServiceObject	object = resolve(_host);
		h = object.host();
		p = object.port();
	} else {
		h = host().c_str();
		p =  port();
	}
	std::string	connectstring = stringprintf("%s:default -h %s -p %hu",
		service.c_str(), h.c_str(), p);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connecting to %s",
		connectstring.c_str());
	return connectstring;
}

bool	ServerName::isDefault() const {
	return isDefaultPort() && (_host == std::string("localhost"));
}

bool	ServerName::isDefaultPort() const {
	return _port == default_port;
}

} // namespace astro
