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
 */
class ServiceObject {
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
	ServiceObject(const std::string& name, int port,
		const std::string host = std::string("localhost"));
	ServiceObject(const std::string& url);

	// convert an object to a string representation
	std::string	toString() const;

	// comparison
	bool	operator<(const ServiceObject& other) const;
};

class ServiceDiscovery;
typedef std::shared_ptr<ServiceDiscovery>	ServiceDiscoveryPtr;

/**
 * \brief A class encapsulating service descovery on different plattforms
 *
 * Linux and the Mac have vastly different implementations for DNS service
 * discovery. To unify service discovery, this class is provided. It cannot
 * directly be instantiated, instead the get method should be used that
 * instatiates an implementation class suitable for the particular plattform.
 */
class ServiceDiscovery {
public:
	typedef enum {
		/**
		 * An instrument service gives information about the URLs
		 * that make up an instrument, i.e. cameras, CCDs, coolers,
		 * guider ports, etc. Instrument services have DNS DS
		 * entries of the form name._instrument._tcp.local
		 */
		INSTRUMENT,
		/**
		 * A task server can be used to control a camera to take
		 * exposures. A task server has a DNS-DS name of the form
		 * <hostname>._tasks._tcp.local
		 */
		TASKS,
		/**
		 * a guiding server can use a and a guiderport to guide a
		 * telescope. The associated DNS-DS name is
		 * <hostname>._guiding._tcp.local
		 */
		GUIDING,
		/**
		 * a images service makes images available to clients under
		 * DNS-DS name <hostname>._images._tcp.local
		 */
		IMAGES
	} service_type;
private:
	service_type	_type;
public:
	service_type	type() const { return _type; }

	static std::string	type_name(service_type t);
	static service_type	type_name(const std::string& n);
protected:
	std::string	dnssd_type;
public:
	ServiceDiscovery(service_type t);
	virtual ~ServiceDiscovery();

	// factory method for an implementation class
	static ServiceDiscoveryPtr	get(service_type t);

	// we keep a set of services we have published and a set of
	// services that have been seen. Both are of type ServiceSet
public:
	typedef std::set<ServiceObject>	ServiceSet;
private:
	ServiceSet	published;
public:
	virtual void	publish(const ServiceObject& object);
	// the set of services that we have resolved
private:
	ServiceSet	services;
public:
	const ServiceSet&	resolve() const;

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


} // namespace discover
} // namespace astro

#endif /* _ServiceDiscovery_h */
