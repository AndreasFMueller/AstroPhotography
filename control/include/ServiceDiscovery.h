/*
 * ServiceDiscovery.h -- classes to encapsulate dns service discover
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ServiceDiscovery_h
#define _ServiceDiscovery_h

#include <string>
#include <memory>
#include <set>
#include <list>
#include <iostream>
#include <future>

namespace astro {
namespace discover {

/**
 * \brief Key for identifying services
 *
 * Services are identified by their name, which must be unique within
 * a domain.
 */
class ServiceKey {
	std::string	_name;
	std::string	_type;
	std::string	_domain;
public:
	const std::string&	name() const { return _name; }
	const std::string&	type() const { return _type; }
	const std::string&	domain() const { return _domain; }
public:
	ServiceKey(const std::string& name, const std::string& type,
		const std::string& domain);
	bool	operator<(const ServiceKey& other) const;
	std::string	toString() const;
};

/**
 * \brief A class encoding the services implemented by an _astro._tcp server
 */
class ServiceSubset {
public:
	typedef enum {
		/**
		 * An instrument service gives information about the URLs
		 * that make up an instrument, i.e. cameras, CCDs, coolers,
		 * guider ports, etc. 
		 */
		INSTRUMENTS = 1,
		/**
		 * A task server can be used to control a camera to take
		 * exposures. 
		 */
		TASKS = 2,
		/**
		 * a guiding server can use a and a guiderport to guide a
		 * telescope.
		 */
		GUIDING = 4,
		/**
		 * a images service makes images available to clients
		 */
		IMAGES = 8
	} service_type;
private:
	int	_services;
	bool	validtype(service_type t) const;
public:
	service_type	string2type(const std::string& name) const;
	std::string	type2string(service_type type) const;

	void	set(service_type type);
	void	set(const std::string& type) { set(string2type(type)); }

	void	unset(service_type type);
	void	unset(const std::string& type) { unset(string2type(type)); }

	bool	has(service_type type) const;
	bool	has(const std::string& type) const {
			return has(string2type(type));
	}

	std::list<std::string>	types() const;
	std::string	toString() const;

	ServiceSubset();
	ServiceSubset(const std::list<std::string>& names);

};

/**
 * \brief Objects encapsulating the information published in dns-ds
 *
 * Service objects describe the services published by a server. We use
 * the following naming convention for the services offered by the system.
 * Every server of the system publishes a service with type _astro._tcp
 * named with the name of the service. But since a server may not offer
 * all functions, it will in addition publish a subtype for each service
 * it publishes. All these service entries will usually have the same
 * port, but we at least allow for them to listen on different ports.
 */
class ServiceObject : public ServiceKey, public ServiceSubset {
	// the _port and _host attributes are available only after resolution
private:
	int	_port;
public:
	int	port() const { return _port; }
	void	port(int p) { _port = p; }
private:
	std::string	_host;
public:
	const std::string&	host() const { return _host; }
	void	host(const std::string& h) { _host = h; }

	// constructors
public:
	ServiceObject(const ServiceKey& key);

	// convert an object to a string representation
	std::string	toString() const;

	// comparison
	bool	operator<(const ServiceObject& other) const;
};

std::ostream&	operator<<(std::ostream& out, const ServiceObject& o);

/**
 * \brief Resolver base class
 */
class ServiceResolver {
protected:
	ServiceKey	_key;
	ServiceObject	_object;
	std::future<ServiceObject>	_resolved;
public:
	ServiceResolver(const ServiceKey& key);
	virtual ~ServiceResolver();
	ServiceObject	resolved();
	virtual ServiceObject	do_resolve() = 0;
};

class ServiceDiscovery;
typedef std::shared_ptr<ServiceDiscovery>	ServiceDiscoveryPtr;

/**
 * \brief A class encapsulating service discovery on different plattforms
 *
 * Linux and the Mac have vastly different implementations for DNS service
 * discovery. To unify service discovery, this class is provided. It cannot
 * directly be instantiated, instead the get method should be used that
 * instatiates an implementation class suitable for the particular plattform.
 */
class ServiceDiscovery {
public:
	ServiceDiscovery();
	virtual ~ServiceDiscovery();

	// factory method for an implementation class
	static ServiceDiscoveryPtr	get();

	// we keep a set of services we have published and a set of
	// services that have been seen. Both are of type ServiceSet
public:
	typedef std::set<ServiceKey>	ServiceKeySet;
private:
	ServiceKeySet	servicekeys;
public:
	const ServiceKeySet&	list() const { return servicekeys; }
protected:
	void	add(const ServiceKey& key);
	void	remove(const ServiceKey& key);
public:
	ServiceObject	find(const std::string& name);
	virtual ServiceObject	find(const ServiceKey& key) = 0;
};

/**
 * \brief Display a list of services
 */
std::ostream&	operator<<(std::ostream& out,
			const ServiceDiscovery::ServiceKeySet& services);

class ServicePublisher;
typedef std::shared_ptr<ServicePublisher>	ServicePublisherPtr;

/**
 * \brief A class encapsulating service publishing on different plattforms
 *
 * The same remarks apply as for the ServiceDiscovery class.
 */
class ServicePublisher : public ServiceSubset {
	std::string	_servername;
public:
	const std::string&	servername() const { return _servername; }
private:
	int	_port;
public:
	int	port() const { return _port; }
public:
	ServicePublisher(const std::string& servername, int port);
	virtual ~ServicePublisher();

	// factory method for an implementation class
	static ServicePublisherPtr	get(const std::string& servername,
						int port);

	virtual void	publish();
};

} // namespace discover
} // namespace astro

#endif /* _ServiceDiscovery_h */
