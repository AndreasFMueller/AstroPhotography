/*
 * AvahiDiscovery.cpp -- Avahi-based service discovery implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include "AvahiDiscovery.h"
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
 * \brief Constructor for the AvahiDiscovery object
 *
 * This constructor also creates the thread and launches it on the avahi_main
 * function, which is just a redirection to the main method of the 
 * AvahiDiscovery object.
 */
AvahiBase::AvahiBase() : simple_poll(NULL), client(NULL) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create AvahiBase object");
	_fut = _prom.get_future();
	_valid = true;
}

/**
 * \brief Destroy the avahi discovery object
 *
 * This destrutctor must cancel the the simple_poll thread
 */
AvahiBase::~AvahiBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiBase object");
	if (client) {
		avahi_client_free(client);
		client = NULL;
	}
	if (simple_poll) {
		avahi_simple_poll_free(simple_poll);
		simple_poll = NULL;
	}
}

/**
 * \brief Test validity
 */
bool	AvahiBase::valid() {
	bool	result;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "is future valid?");
	try {
		result = _fut.get();
	} catch (const std::exception& x) {
		_valid = false;
		result = _valid;
	}
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main_startup complete");
	return true;
fail:
	_valid = false;
	_prom.set_value(false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main_startup failed");
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
		_valid = false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "client callback completed");
}

} // namespace discover
} // namespace astro
