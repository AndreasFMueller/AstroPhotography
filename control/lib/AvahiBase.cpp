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
AvahiBase::AvahiBase() : simple_poll(NULL), client(NULL),
	thread(avahi_main, this) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create AvahiBase object");
	_valid = _prom.get_future();
}

/**
 * \brief Destroy the avahi discovery object
 *
 * This destrutctor must cancel the the simple_poll thread
 */
AvahiBase::~AvahiBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiBase");
	if (valid()) {
		avahi_simple_poll_quit(simple_poll);
	}
	// wait for the thread to terminate
	debug(LOG_DEBUG, DEBUG_LOG, 0, "join the thread");
	thread.join();

	if (client) {
		avahi_client_free(client);
		client = NULL;
	}
	if (simple_poll) {
		avahi_simple_poll_free(simple_poll);
		simple_poll = NULL;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiBase object");
}

/**
 * \brief Test validity
 */
bool	AvahiBase::valid() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "is future valid?");
	bool	result = _valid.get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got value");
	return result;
}

/**
 * \brief Client callback implementation, trampoline function
 */
static void	client_callback(AvahiClient *client, AvahiClientState state,
			void *userdata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "client callback trampoline");
	AvahiBase	*base = (AvahiBase *)userdata;
	base->client_callback(client, state);
}

/**
 * \brief Startup sequence for the main method
 */
bool	AvahiBase::main_startup() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "base main program started %p",
		this);

	// create Avahi simple poll object
	simple_poll = avahi_simple_poll_new();
	if (NULL == simple_poll) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"failed to create simple poll object");
		goto fail;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simple poll created");

	// create avahi client
	int	error;
	client = avahi_client_new(avahi_simple_poll_get(simple_poll),
		(AvahiClientFlags)0, discover::client_callback, this, &error);
	if (NULL == client) {
		debug(LOG_ERR, DEBUG_LOG, 0, "failed to create client: %s",
			avahi_strerror(error));
		goto fail;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "avahi client created @ %p", client);

	// event loop for the poll
	_prom.set_value(true);
	return true;
fail:
	_prom.set_value(false);
	return false;
}

/**
 * \brief callback reporting state changes in the avahi client
 */
void	AvahiBase::client_callback(AvahiClient *client,
		AvahiClientState state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AvahiBase::client_callback");
	assert(client);
	if (state == AVAHI_CLIENT_FAILURE) {
		debug(LOG_ERR, DEBUG_LOG, 0, "server connection failure: %s",
			avahi_strerror(avahi_client_errno(client)));
		avahi_simple_poll_quit(simple_poll);
		_prom.set_value(false);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "client callback completed");
}

} // namespace discover
} // namespace astro
