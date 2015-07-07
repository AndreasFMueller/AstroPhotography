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
#include <iostream>

namespace astro {
namespace discover {


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
class ServiceObject {
public:
	typedef enum {
		/**
		 * An instrument service gives information about the URLs
		 * that make up an instrument, i.e. cameras, CCDs, coolers,
		 * guider ports, etc. Instrument services have DNS DS
		 * entries of the form
		 * <servername>._instruments._astro._tcp.local
		 */
		INSTRUMENTS,
		/**
		 * A task server can be used to control a camera to take
		 * exposures. A task server has a DNS-DS name of the form
		 * <servername>._tasks._astro._tcp.local
		 */
		TASKS,
		/**
		 * a guiding server can use a and a guiderport to guide a
		 * telescope. The associated DNS-DS name is
		 * <servername>._guiding._astro._tcp.local
		 */
		GUIDING,
		/**
		 * a images service makes images available to clients under
		 * DNS-DS name <servername>._images._astro._tcp.local
		 */
		IMAGES
	} service_type;
private:
	service_type	_type;
public:
	service_type	type() const { return _type; }

	static std::string	type_name(service_type t);
	static service_type	type_name(const std::string& n);
private:
	std::string	_name;
public:
	const std::string&	name() const { return _name; }
	void	name(const std::string& n) { _name = n; }
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
public:
	ServiceObject(const std::string& name, service_type type);
	ServiceObject(const std::string& name, const std::string& tn);

	// convert an object to a string representation
	std::string	toString() const;

	// comparison
	bool	operator<(const ServiceObject& other) const;
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
	typedef std::set<ServiceObject>	ServiceSet;
private:
	ServiceSet	services;
public:
	const ServiceSet&	resolve() const;
	const ServiceObject&	find(const std::string& name) const;

	// methods to add and remove services when the implementation
	// detects a change
public:
	void	add(const ServiceObject& so);
	void	remove(const std::string& name);
};

/**
 * \brief Display a list of services
 */
std::ostream&	operator<<(std::ostream& out,
			const ServiceDiscovery::ServiceSet& services);

class ServicePublisher;
typedef std::shared_ptr<ServicePublisher>	ServicePublisherPtr;

/**
 * \brief A class encapsulating service publishing on different plattforms
 *
 * The same remarks apply as for the ServiceDiscovery class.
 */
class ServicePublisher {
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

	// we keep a set of services we have published and a set of
	// services that have been seen. Both are of type ServiceSet
public:
	typedef std::set<ServiceObject::service_type>	ServiceTypeSet;
protected:
	ServiceTypeSet	published;
public:
	void	add(ServiceObject::service_type type);
	void	revoke(ServiceObject::service_type type);
	virtual void	publish();
};

} // namespace discover
} // namespace astro

#endif /* _ServiceDiscovery_h */
