/*
 * AvahiDiscovery.cpp -- Avahi-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AvahiDiscovery.h>
#include <AstroDebug.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>

namespace astro {
namespace discover {

/**
 * \brief Trampoline function to run the threads main method
 */
static void	avahi_main(AvahiBase *base) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "call the virtual main method");
	base->main();
}

/**
 * \brief Constructor for the AvahiDiscovery object
 *
 * This constructor also creates the thread and launches it on the avahi_main
 * function, which is just a redirection to the main method of the 
 * AvahiDiscovery object.
 */
AvahiBase::AvahiBase() : _valid(false), thread(avahi_main, this) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create AvahiBase object");
}

/**
 * \brief Destroy the avahi discovery object
 *
 * This destrutctor must cancel the the simple_poll thread
 */
AvahiBase::~AvahiBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "join the thread");
	if (valid()) {
		avahi_simple_poll_quit(simple_poll);
	}
	// wait for the thread to terminate
	thread.join();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiBase object");
}

} // namespace discover
} // namespace astro
