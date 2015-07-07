/*
 * AvahiPublisher.cpp -- Avahi-based service discovery implementation
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

void	AvahiPublisher::entry_group_callback(AvahiEntryGroup *g,
		AvahiEntryGroupState state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"AvahiPublisher::entry_group_callback %p, %p, %d", this, g, state);

	// remember the group, if we don't have it already
	if (!(group == g || group == NULL)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "preconditions not met");
		throw std::runtime_error("preconditions not met");
	}
	group = g;

	switch (state) {
	case AVAHI_ENTRY_GROUP_UNCOMMITED:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "uncommited");
		break;
	case AVAHI_ENTRY_GROUP_REGISTERING:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "registering");
		break;
	case AVAHI_ENTRY_GROUP_ESTABLISHED:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "established");
		break;
	case AVAHI_ENTRY_GROUP_COLLISION:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "collision");
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
 * \brief Client callback implementation
 */
static void	client_callback(AvahiClient *client, AvahiClientState state,
			void *userdata) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "client_callback called");
	AvahiPublisher	*discovery = (AvahiPublisher *)userdata;
	discovery->client_callback(client, state);
}

void	AvahiPublisher::client_callback(AvahiClient *client,
		AvahiClientState state) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AvahiPublisher::client_callback");
	assert(client);
	switch (state) {
	case AVAHI_CLIENT_FAILURE:
		debug(LOG_ERR, DEBUG_LOG, 0, "server connection failure: %s",
			avahi_strerror(avahi_client_errno(client)));
		avahi_simple_poll_quit(simple_poll);
		break;

	case AVAHI_CLIENT_S_RUNNING:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "client (%p) is running",
			client);
		// create services
		create_services(client);
		break;

	case AVAHI_CLIENT_S_COLLISION:
		// just perform a reset in this case

	case AVAHI_CLIENT_S_REGISTERING:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "registering");
		// if the group exist, reset it
		if (group) {
			avahi_entry_group_reset(group);
		}
		break;

	case AVAHI_CLIENT_CONNECTING:
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
	if (published.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no services to add");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding services (%d)",
		published.size());
	// build an array for the text records
	AvahiStringList	*strlist = NULL;
	for (ServiceTypeSet::iterator j = published.begin();
		j != published.end(); j++) {
		switch (*j) {
		case ServiceObject::INSTRUMENTS:
			strlist = avahi_string_list_add(strlist, "instruments");
			break;
		case ServiceObject::TASKS:
			strlist = avahi_string_list_add(strlist, "tasks");
			break;
		case ServiceObject::GUIDING:
			strlist = avahi_string_list_add(strlist, "guiding");
			break;
		case ServiceObject::IMAGES:
			strlist = avahi_string_list_add(strlist, "images");
			break;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created stringlist of %d elements",
		avahi_string_list_length(strlist));

	// first we have to add the service with the name of the object
	int	rc;
	rc = avahi_entry_group_add_service_strlst(group, AVAHI_IF_UNSPEC,
		AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,
		servername().c_str(), "_astro._tcp",
		NULL, NULL, port(), strlist);

	//avahi_string_list_free(strlist);

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

void	AvahiPublisher::modify_callback(AvahiTimeout *e) {
	if (avahi_client_get_state(client) == AVAHI_CLIENT_S_RUNNING) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "modify published services");
		if (group)
			avahi_entry_group_reset(group);
		create_services(client);
	}
}

void	AvahiPublisher::publish() {
	// rebuild the services
	struct timeval	tv;
	avahi_simple_poll_get(simple_poll)->timeout_new(
		avahi_simple_poll_get(simple_poll),
		avahi_elapse_time(&tv, 0, 100),
		discover::modify_callback, this);
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
	_valid = false;

	// create avahi client objects
	client = NULL;
	simple_poll = NULL;

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
	_valid = true;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "running simple_poll loop");
	avahi_simple_poll_loop(simple_poll);

	_valid = false;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "main program for discover %p complete",
		this);
fail:
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
