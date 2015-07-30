/*
 * AvahiDiscovery.h -- Avahi-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AvahiDiscovery_h
#define _AvahiDiscovery_h

#include <AstroDiscovery.h>
#include <thread>
#include <avahi-common/simple-watch.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>

namespace astro {
namespace discover {

/**
 * \brief ServiceSubset classes to convert type to 
 */
class AvahiServiceSubset : public ServiceSubset {
public:
	AvahiServiceSubset() { }
	AvahiServiceSubset(const std::list<std::string>& names);
	
	static AvahiStringList	*stringlist(const ServiceSubset& s);
	AvahiStringList	*stringlist() const;
};

/**
 * \brief Avahi base class
 *
 * The Avahi base class handles the thread management
 */
class	AvahiBase {
protected:
	std::promise<bool>	_prom;
	std::shared_future<bool>	_fut;
	bool	_valid;
public:
	bool	valid();
protected:
	AvahiSimplePoll	*simple_poll;
	AvahiClient	*client;
	bool	main_startup();
private:
	// make AvahiDiscovery uncopiable
	AvahiBase(const AvahiBase& other);
	AvahiBase&	operator=(const AvahiBase& other);
public:
	AvahiBase();
	virtual ~AvahiBase();
	// virtual main function, needs to be implemented in derived class
	virtual void	client_callback(AvahiClient *client,
				AvahiClientState state);
};

/**
 * \brief Thread encapsulation for Avahi
 */
class AvahiThread : public AvahiBase {
protected:
	std::thread	*thread;
private:
	bool	running;
	std::mutex	mtx;
	AvahiThread(const AvahiThread& other);
	AvahiThread&	operator=(const AvahiThread& other);
public:
	virtual void	main() = 0;
	AvahiThread();
	virtual ~AvahiThread();
protected:
	virtual void	start();
};

/**
 * \brief resolver class for Avahi implementation
 */
class AvahiResolver : public ServiceResolver {
	AvahiClient	*_client;
	// a std::future variable is used as the handshake mechanism
	// to signal completion of the resolution
	std::shared_ptr<std::future<bool> >	fut;
	std::shared_ptr<std::promise<bool> >	prom;
public:
	AvahiResolver(const ServiceKey& key, AvahiClient *client);
	~AvahiResolver();
	virtual ServiceObject	do_resolve();

	// callback related to name resolution
	void	resolve_callback(
			AvahiServiceResolver *resolver,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const char *name,
			const char *type,
			const char *domain,
			const char *host_name,
			const AvahiAddress *address,
			uint16_t port,
			AvahiStringList *txt,
			AvahiLookupResultFlags flags);
};

/**
 * \brief Service discovery using Avahi*
 */
class AvahiDiscovery : public ServiceDiscovery, public AvahiThread {
public:
	AvahiDiscovery();
	virtual ~AvahiDiscovery();
	virtual void	main();

public:
	// callback related to browsing
	void     browse_callback(
			AvahiServiceBrowser *sb,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiBrowserEvent event,
			const char *name,
			const char *type,
			const char *domain,
			AvahiLookupResultFlags flags);


	virtual ServiceObject	find(const ServiceKey& key);
	void	start() { AvahiThread::start(); }
};

/**
 * \brief Service publishing using Avahi
 */
class AvahiPublisher : public ServicePublisher, public AvahiThread {
public:
	AvahiPublisher(const std::string& servername, int port);
	virtual ~AvahiPublisher();
	virtual void	main();

public:
	AvahiEntryGroup	*group;
	void	entry_group_callback(AvahiEntryGroup *g,
			AvahiEntryGroupState state);

	// callback related to client activities
	void	client_callback(AvahiClient *c, AvahiClientState state);
	void	modify_callback(AvahiTimeout *e);

	virtual void	publish();

	// registration methods
	void	create_services(AvahiClient *client);
	void	add_service_objects(AvahiClient *client);
};

} // namespace discover
} // namespace astro

#endif /* _AvahiDiscovery_h */
