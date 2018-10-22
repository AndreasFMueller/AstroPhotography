/*
 * AvahiPublisher.cpp -- Avahi-based service discovery implementation
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
#include <avahi-common/timeval.h>

namespace astro {
namespace discover {

/**
 * \brief Constructor for the AvahiPublisher object
 *
 * This constructor also creates the thread and launches it on the avahi_main
 * function, which is just a redirection to the main method of the 
 * AvahiPublisher object.
 */
AvahiPublisher::AvahiPublisher(const std::string& servername, int port)
	: ServicePublisher(servername, port) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create AvahiPublisher object");
	group = NULL;
}

/**
 * \brief Destroy the avahi discovery object
 *
 * This destrutctor must cancel the the simple_poll thread
 */
AvahiPublisher::~AvahiPublisher() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy AvahiPublisher object");
}

/**
 * \brief Callback for group related events
 */
static void	entry_group_callback(AvahiEntryGroup *g,
			AvahiEntryGroupState state,
			void *userdata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "entry_group_callback %d, userdata = %p",
		state, userdata);
	AvahiPublisher	*discovery = (AvahiPublisher *)userdata;
	discovery->entry_group_callback(g, state);
}

/**
 * \brief Callback for the entry group
 *
 * When this callback is called, then the entry group is ready to receive
 * service entries. We will then save the group
 */
void	AvahiPublisher::entry_group_callback(AvahiEntryGroup *g,
		AvahiEntryGroupState state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"AvahiPublisher::entry_group_callback %p, %p, %d", this, g,
		state);

	// remember the group, if we don't have it already
	if (!(group == g || group == NULL)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "preconditions not met");
		throw std::runtime_error("preconditions not met");
	}
	group = g;

	switch (state) {
	case AVAHI_ENTRY_GROUP_UNCOMMITED:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d group uncommited", state);
		break;
	case AVAHI_ENTRY_GROUP_REGISTERING:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d group registering", state);
		break;
	case AVAHI_ENTRY_GROUP_ESTABLISHED:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d group established", state);
		break;
	case AVAHI_ENTRY_GROUP_COLLISION:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d group collision", state);
		break;
	case AVAHI_ENTRY_GROUP_FAILURE:
		debug(LOG_ERR, DEBUG_LOG, 0, "error during group operation: %s",
			avahi_strerror(avahi_client_errno(client)));
		avahi_simple_poll_quit(simple_poll);
		break;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "entry_group_callback completed");
}

/**
 * \brief callback reporting state changes in the avahi client
 */
void	AvahiPublisher::client_callback(AvahiClient *client,
		AvahiClientState state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AvahiPublisher::client_callback");
	// handle failure
	AvahiBase::client_callback(client, state);
	assert(client);
	switch (state) {
	case AVAHI_CLIENT_S_RUNNING:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "client (%p) is running",
			client);
		// create services
		//create_services(client);
		break;

	case AVAHI_CLIENT_S_COLLISION:
		// just perform a reset in this case

	case AVAHI_CLIENT_S_REGISTERING:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "registering");
		// if the group exist, reset it
		if (group) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "resetting the group");
			avahi_entry_group_reset(group);
		}
		break;

	case AVAHI_CLIENT_CONNECTING:
		break;
	case AVAHI_CLIENT_FAILURE:
		// not used, as this is handled in AvahiBase::client_callback
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "client callback completed");
}

/**
 * \brief Create services in the Avahi group
 */
void	AvahiPublisher::create_services(AvahiClient *client) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating services: client = %p",
		client);
	// if the group does not exist yet, create it now
	if (NULL == group) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "creating entry group");
		AvahiEntryGroup	*g  = avahi_entry_group_new(client,
				discover::entry_group_callback, this);
		if (NULL == g) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot create the group: %s",
				avahi_strerror(avahi_client_errno(client)));
			goto fail;
		}
	}

	if (avahi_entry_group_is_empty(group)) {
		add_service_objects(client);
	}
	return;
fail:
	debug(LOG_ERR, DEBUG_LOG, 0, "failed to create services");
	return;
}

/**
 * \brief Add all service objects to the group
 */
void	AvahiPublisher::add_service_objects(AvahiClient *client) {
	// build an array for the text records
	AvahiStringList	*strlist = AvahiServiceSubset::stringlist(*this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d txt entries",
		avahi_string_list_length(strlist));

	// first we have to add the service with the name of the object
	int	rc;
	rc = avahi_entry_group_add_service_strlst(group, AVAHI_IF_UNSPEC,
		AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,
		servername().c_str(), "_astro._tcp",
		NULL, NULL, port(), strlist);

	avahi_string_list_free(strlist);

	if (rc == AVAHI_ERR_COLLISION) {
		debug(LOG_ERR, DEBUG_LOG, 0, "name collision, exiting");
		goto fail;
	}

	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot add service: %s",
			avahi_strerror(avahi_client_errno(client)));
		goto fail;
	}

	// commit the group
	debug(LOG_DEBUG, DEBUG_LOG, 0, "commiting the group");
	rc = avahi_entry_group_commit(group);
	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot commit: %s",
			avahi_strerror(rc));
	}
	return;

fail:
	avahi_simple_poll_quit(simple_poll);
	return;
}

static void	modify_callback(AvahiTimeout *e, void *userdata) {
	AvahiPublisher	*publisher = (AvahiPublisher *)userdata;
	publisher->modify_callback(e);
}

void	AvahiPublisher::modify_callback(AvahiTimeout * /* e */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modify_callback called");
	if (avahi_client_get_state(client) == AVAHI_CLIENT_S_RUNNING) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "modify published services");
		if (group) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "resetting the group");
			avahi_entry_group_reset(group);
		}
		create_services(client);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modify_callback complete");
}

void	AvahiPublisher::publish() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "publish %s:%d %s",
		servername().c_str(), port(),
		ServiceSubset::toString().c_str());
	start();
	if (!valid()) {
		throw std::runtime_error("publishing thread failed");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread is still valid");
	// rebuild the services
	struct timeval	tv;
	avahi_simple_poll_get(simple_poll)->timeout_new(
		avahi_simple_poll_get(simple_poll),
		avahi_elapse_time(&tv, 0, 100),
		discover::modify_callback, this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "publish completed");
	ServicePublisher::publish();
}

/**
 * \brief Main method of the AvahiPublisher object
 *
 * This method launches the main loop and also handles cleanup of avahi
 * related objects when the thread exits
 */
void	AvahiPublisher::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main program started for publishing %p",
		this);
	if (!main_startup()) {
		return;
	}

	// event loop for the poll
	debug(LOG_DEBUG, DEBUG_LOG, 0, "running simple_poll loop");
	avahi_simple_poll_loop(simple_poll);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "main program for discover %p complete",
		this);
	_valid = false;
	if (client) {
		avahi_client_free(client);
		client = NULL;
	}
	if (simple_poll) {
		avahi_simple_poll_free(simple_poll);
		simple_poll = NULL;
	}
}

} // namespace discover
} // namespace astro
