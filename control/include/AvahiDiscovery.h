/*
 * AvahiDiscovery.h -- Avahi-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AvahiDiscovery_h
#define _AvahiDiscovery_h

#include <ServiceDiscovery.h>
#include <thread>
#include <avahi-common/simple-watch.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>

namespace astro {
namespace discover {

/**
 * \brief Avahi base class
 *
 * The Avahi base class handles the thread management
 */
class	AvahiBase {
protected:
	bool	_valid;
public:
	bool	valid() const { return _valid; }
protected:
	std::thread	thread;
	AvahiSimplePoll	*simple_poll;
private:
	// make AvahiDiscovery uncopiable
	AvahiBase(const AvahiBase& other);
	AvahiBase&	operator=(const AvahiBase& other);
public:
	AvahiBase();
	virtual ~AvahiBase();
	// virtual main function, needs to be implemented in derived class
	virtual void	main() = 0;
};

/**
 * \brief Service discovery using Avahi*
 */
class AvahiDiscovery : public ServiceDiscovery, public AvahiBase {
public:
	AvahiDiscovery();
	virtual ~AvahiDiscovery();
	virtual void	main();

public:
	// callback related to client activities
	void	client_callback(AvahiClient *c, AvahiClientState state);

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
 * \brief Service publishing using Avahi
 */
class AvahiPublisher : public ServicePublisher, public AvahiBase {
public:
	AvahiPublisher(const std::string& servername, int port);
	virtual ~AvahiPublisher();
	virtual void	main();

public:
	AvahiEntryGroup	*group;
	void	entry_group_callback(AvahiEntryGroup *g,
			AvahiEntryGroupState state);
	AvahiClient	*client;

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
