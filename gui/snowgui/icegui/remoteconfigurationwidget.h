/*
 * remoteconfigurationwidget.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#ifndef _remoteconfigurationwidget_h
#define _remoteconfigurationwidget_h

#include <configurationwidget.h>
#include <types.h>

namespace snowgui {

class remoteconfigurationwidget : public configurationwidget {

	snowstar::ConfigurationPrx	_configuration;

public:
	explicit remoteconfigurationwidget(QWidget *parent = nullptr);
	~remoteconfigurationwidget();

	// set the proxy configuration
	snowstar::ConfigurationPrx	configuration() {
		return _configuration;
	}

	void	setConfiguration(snowstar::ConfigurationPrx c) {
		_configuration = c;
		filltable();
	}

	// interface to access configuration data
	virtual std::list<astro::config::ConfigurationKey>	listkeys();
	virtual bool	has(const astro::config::ConfigurationKey& key);
	virtual std::string	description(
				const astro::config::ConfigurationKey& key);
	virtual std::string	value(
				const astro::config::ConfigurationKey& key);
	virtual void	set(const astro::config::ConfigurationKey& key,
				const std::string& value);
	virtual void	remove(const astro::config::ConfigurationKey& key);

};

} // namespace snowgui

#endif /* _remoteconfigurationwidget_h */
