/*
 * remoteconfigurationwidget.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include <remoteconfigurationwidget.h>
#include <IceConversions.h>

namespace snowgui {

remoteconfigurationwidget::remoteconfigurationwidget(QWidget *parent)
	: configurationwidget(parent) {
}

remoteconfigurationwidget::~remoteconfigurationwidget() {
}

// set the proxy configuration

// interface to access configuration data
std::list<astro::config::ConfigurationKey>	remoteconfigurationwidget::listkeys() {
	std::list<astro::config::ConfigurationKey>	keys;
	if (!_configuration) {
		return keys;
	}
	snowstar::ConfigurationKeyList	list = _configuration->registeredKeys();
	std::for_each(list.begin(), list.end(),
		[&keys](const snowstar::ConfigurationKey& key) mutable {
			keys.push_back(snowstar::convert(key));
		}
	);
	return keys;
}


bool	remoteconfigurationwidget::has(const astro::config::ConfigurationKey& key) {
	if (!_configuration) {
		return false;
	}
	return _configuration->has(snowstar::convert(key));
}


std::string	remoteconfigurationwidget::description(const astro::config::ConfigurationKey& key) {
	if (!_configuration) {
		return std::string();
	}
	return _configuration->description(snowstar::convert(key));
}


std::string	remoteconfigurationwidget::value(const astro::config::ConfigurationKey& key) {
	if (!_configuration) {
		return std::string();
	}
	snowstar::ConfigurationItem	item
		= _configuration->get(snowstar::convert(key));
	return item.value;
}


void	remoteconfigurationwidget::set(const astro::config::ConfigurationKey& key,
			const std::string& value) {
	if (!_configuration) {
		return;
	}
	astro::config::ConfigurationEntry	entry(key, value);
	_configuration->set(snowstar::convert(entry));
}


void	remoteconfigurationwidget::remove(const astro::config::ConfigurationKey& key) {
	if (!_configuration) {
		return;
	}
	_configuration->remove(snowstar::convert(key));
}

} // namespace snowgui
