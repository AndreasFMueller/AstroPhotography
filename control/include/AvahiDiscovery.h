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

namespace astro {
namespace discover {

class AvahiDiscovery : public ServiceDiscovery {
	std::thread	thread;
	bool	_valid;
public:
	bool	valid() const { return _valid; }
	AvahiSimplePoll	*simple_poll;
private:
	// make AvahiDiscovery uncopiable
	AvahiDiscovery(const AvahiDiscovery& other);
	AvahiDiscovery&	operator=(const AvahiDiscovery& other);
public:
	AvahiDiscovery(service_type t);
	virtual ~AvahiDiscovery();
	void	main();
};

} // namespace discover
} // namespace astro

#endif /* _AvahiDiscovery_h */
