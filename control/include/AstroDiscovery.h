/*
 * AstroDiscovery.h -- classes to encapsulate dns service discover
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDiscovery_h
#define _AstroDiscovery_h

#include <string>
#include <memory>
#include <set>
#include <list>
#include <iostream>
#include <future>
#include <AstroPersistence.h>
#include <mutex>
#include <condition_variable>
#include <AstroDevice.h>

namespace astro {
namespace discover {

/**
 * \brief Class to locate the service
 */
class ServiceLocation {
	std::string	_servicename;
	unsigned short	_port;
	unsigned short	_sslport;
	bool	_ssl;
private:
	// prevent copying
	ServiceLocation(const ServiceLocation& other);
	ServiceLocation&	operator=(const ServiceLocation& other);
public:
	const std::string&	servicename() const { return _servicename; }
	void	servicename(const std::string& s) { _servicename = s; }

	unsigned short	port() const { return _port; }
	void	port(unsigned short p) { _port = p; }

	unsigned short	sslport() const { return _sslport; }
	void	sslport(unsigned short s) { _sslport = s; }

	bool	ssl() const { return _ssl; }
	void	ssl(bool s) { _ssl = s; }

	ServiceLocation() : _port(0), _sslport(0), _ssl(false) { }
	void	locate();
public:
static ServiceLocation&	get();
};

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
	int	_interface;
	int	_protocol;
public:
	const std::string&	name() const { return _name; }
	const std::string&	type() const { return _type; }
	const std::string&	domain() const { return _domain; }

	ServiceKey(const std::string& name, const std::string& type,
		const std::string& domain);
	ServiceKey() { }

	int	interface() const { return _interface; }
	void	interface(int i) { _interface = i; }
	int	protocol() const { return _protocol; }
	void	protocol(int p) { _protocol = p; }

	bool	operator<(const ServiceKey& other) const;
	bool	operator==(const ServiceKey& other) const;
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
	static service_type	string2type(const std::string& name);
	static std::string	type2string(service_type type);

	void	set(service_type type);
	void	set(const std::string& type) { set(string2type(type)); }
	void	set(const std::list<std::string>& names);

	void	unset(service_type type);
	void	unset(const std::string& type) { unset(string2type(type)); }
	void	unset(const std::list<std::string>& names);

	bool	has(service_type type) const;
	bool	has(const std::string& type) const {
			return has(string2type(type));
	}

	void	clear() { _services = 0; }

	std::list<std::string>	types() const;
	std::string	toString() const;

	std::string	txtrecord() const;
	static std::list<std::string>	txtparse(const std::string& txt);

	ServiceSubset();
	ServiceSubset(const std::list<std::string>& names);
	ServiceSubset(const std::string& txt);

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

	// get an ICE connection string
	std::string	connect(const std::string& service) const;

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
	std::shared_future<ServiceObject>	_resolved;
	// locking to ensure resolution is initiated only once
	std::mutex	resolvinglock;
	bool		resolving;
public:
	ServiceResolver(const ServiceKey& key);
	virtual ~ServiceResolver();
	ServiceObject	resolved();
	void	resolve();
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

	// start it
	virtual void	start() = 0;

	// we keep a set of services we have published and a set of
	// services that have been seen. Both are of type ServiceSet
public:
	typedef std::set<ServiceKey>	ServiceKeySet;
private:
	std::recursive_mutex	servicelock;
	std::condition_variable_any	servicecondition;
	ServiceKeySet	servicekeys;
public:
	const ServiceKeySet&	list() const { return servicekeys; }
protected:
	void	add(const ServiceKey& key);
	void	remove(const ServiceKey& key);
public:
	bool	has(const std::string& name);
	bool	has(const ServiceKey& key);
	ServiceKey	waitfor(const std::string& name);
	ServiceKey	find(const std::string& name);
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
 * The same remarks apply as for the AstroDiscovery class.
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
private:
static	void	add_published(const std::string& name);
static	void	remove_published(const std::string& name);
public:
static bool	ispublished(const std::string& name);
};

class Instrument;

/**
 * \brief Key class for access to instruments and components
 */
class	InstrumentComponentKey {
public:
	typedef enum { 
		AdaptiveOptics = 0,
		Camera = 1,
		CCD = 2,
		Cooler = 3,
		GuiderCCD = 4,
		GuiderPort = 5,
		FilterWheel = 6,
		Focuser = 7,
		Mount = 8
	} Type;
	std::string	_name;
	Type	_type;
	int	_index;
public:
	const std::string&	name() const { return _name; }
	std::string&	name() { return _name; }
	void	name(const std::string& n) { _name = n; }

	Type	type() const { return _type; }
	Type&	type() { return _type; }
	void	type(Type t) { _type = t; }

	static std::string	type2string(Type t);
	static Type	string2type(const std::string& tn);

	int	index() const { return _index; }
	int&	index() { return _index; }
	void	index(int i) { _index = i; }

	InstrumentComponentKey(const std::string& name, Type type,
		int index = -1) : _name(name), _type(type), _index(index) {
	}
	InstrumentComponentKey(Type type, int index = -1)
		: _type(type), _index(index) {
	}
	InstrumentComponentKey() : _type(CCD), _index(0) {
	}
	bool	operator<(const InstrumentComponentKey& other) const {
		if (_name < other._name) { return true; }
		if (_name > other._name) { return false; }
		if (_type < other._type) { return true; }
		if (_type > other._type) { return false; }
		return (_index < other._index);
	}
};

/**
 * \brief Instrument Component
 */
class InstrumentComponent : public InstrumentComponentKey {
private:
	std::string	_servicename;
	std::string	_deviceurl;
public:
	const std::string&	servicename() const { return _servicename; }
	void	servicename(const std::string& s) { _servicename = s; }

	const std::string&	deviceurl() const { return _deviceurl; }
	void	deviceurl(const std::string& d) { _deviceurl = d; }

	InstrumentComponent(const std::string& instrumentname,
		InstrumentComponentKey::Type type,
		const std::string& servicename, const std::string& deviceurl);
	InstrumentComponent(const InstrumentComponentKey& key,
		const std::string& servicename, const std::string& deviceurl);
	std::string	toString() const;
	DeviceName	remoteName() const;
};

/**
 * \brief Instrument Property 
 */
class InstrumentProperty {
	std::string	_instrument;
	std::string	_property;
	std::string	_value;
	std::string	_description;
public:
	const std::string&	instrument() const { return _instrument; }
	void	instrument(const std::string& i) { _instrument = i; }
	const std::string&	property() const { return _property; }
	void	property(const std::string& p) { _property = p; }
	const std::string&	value() const { return _value; }
	void	value(const std::string& v) { _value = v; }
	const std::string&	description() const { return _description; }
	void	description(const std::string& d) { _description = d; }
	std::string	toString() const;
};
typedef std::list<InstrumentProperty>	InstrumentPropertyList;

class Instrument;
typedef std::shared_ptr<Instrument>	InstrumentPtr;
/**
 * \brief Instrument abstraction
 */
class Instrument {
	std::string	_name;
public:
	const std::string&	name() const { return _name; }
	Instrument(const std::string& name) : _name(name) { }
private:
	void	add(std::list<InstrumentComponent>& l,
			InstrumentComponent::Type type);
public:
	// get a component
	virtual InstrumentComponent	get(InstrumentComponent::Type type,
						int index) = 0;

	virtual int	nComponentsOfType(InstrumentComponentKey::Type type) = 0;
	virtual int	indexOf(InstrumentComponentKey::Type type,
				const std::string& deviceurl) = 0;
	virtual int	add(const InstrumentComponent& component) = 0;
	virtual void	update(const InstrumentComponent& component) = 0;
	virtual void	remove(InstrumentComponentKey::Type type, int index) = 0;
	typedef std::list<InstrumentComponent>	ComponentList;
	ComponentList	list(InstrumentComponentKey::Type type);
	ComponentList	list();

	// properties
	virtual int	addProperty(const InstrumentProperty& property) = 0;
	virtual bool	hasProperty(const std::string& property) = 0;
	virtual InstrumentProperty	getProperty(const std::string& property) = 0;
	virtual void	updateProperty(const InstrumentProperty& property) = 0;
	virtual void	removeProperty(const std::string& property) = 0;
	typedef std::list<std::string>	PropertyNames;
	virtual PropertyNames	getPropertyNames() = 0;
	virtual InstrumentPropertyList	getProperties() = 0;

	// simplified property value access
	int	getInt(const std::string& name);
	double	getDouble(const std::string& name);
	std::string	getString(const std::string& name);
};

class InstrumentList : public std::list<std::string> {
public:
	InstrumentList() { }
	InstrumentList(const std::list<std::string>& list);
};

/**
 * \brief Instrument Backend
 */
class InstrumentBackend {
public:
	InstrumentBackend();
	InstrumentBackend(astro::persistence::Database database);
	// static methods to get information about available 
	static InstrumentList	names();
	static InstrumentPtr	get(const std::string& name);
	static bool	has(const std::string& name);
	static void	remove(const std::string& name);
};

} // namespace discover
} // namespace astro

#endif /* _AstroDiscovery_h */
