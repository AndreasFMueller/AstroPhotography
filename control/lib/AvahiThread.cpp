/*
 * AvahiThread.cpp -- Avahi-based service discovery implementation
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
static void	avahi_main(AvahiThread *base) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "call the virtual main method");
	/*
	 * XXX There is a problem here: we cannot call the virtual function
	 * XXX unless the derived constructors have completed, so this
	 * XXX is bound lead to a race condition problem.
	 */
	base->main();
}

/**
 * \brief Constructor for the AvahiDiscovery object
 *
 * This constructor also creates the thread and launches it on the avahi_main
 * function, which is just a redirection to the main method of the 
 * AvahiDiscovery object.
 */
AvahiThread::AvahiThread() : thread(avahi_main, this) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create AvahiThread object");
}

/**
 * \brief Destroy the avahi discovery object
 *
 * This destrutctor must cancel the the simple_poll thread
 */
AvahiThread::~AvahiThread() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiThread");
	if (valid()) {
		if (simple_poll) {
			avahi_simple_poll_quit(simple_poll);
		}
	}
	// wait for the thread to terminate
	debug(LOG_DEBUG, DEBUG_LOG, 0, "join the thread");
	thread.join();
}

} // namespace discover
} // namespace astro
