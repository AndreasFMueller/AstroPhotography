/*
 * CommunicatorSingleton.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <IceUtil/UUID.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <mutex>

namespace snowstar {

static bool	initialized = false;
static bool	connected = false;
Ice::CommunicatorPtr	CommunicatorSingleton::_communicator;

/**
 * \brief Create the Communicator singleton
 *
 * \param argc		number of arguments
 * \param argv		array of argument strings
 */
CommunicatorSingleton::CommunicatorSingleton(int& argc, char *argv[]) {
	if (initialized) {
		std::string	msg("communicator already initialized");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// extract properties from the command line
	Ice::PropertiesPtr props = Ice::createProperties(argc, argv);

	// dont' ever close connections
	props->setProperty("Ice.ACM.Close", "0");

	// the large message size is required because we have cases
	// where we transfer entire images as messages
	props->setProperty("Ice.MessageSizeMax", "65536");

	// large image files should be compressed, and because the network
	// is slow, it is OK to trade off some CPU cycles for this
	props->setProperty("Ice.Compression.Level", "5");

	// abort on null handle errors
	props->setProperty("Ice.NullHandleAbort", "1");

	// thread pool jproperties
	props->setProperty("Ice.ThreadPool.Server.SizeMax", "15");
	props->setProperty("Ice.ThreadPool.Client.SizeMax", "15");

	Ice::InitializationData	id;
	id.properties = props;

	// initialize the communicator
	_communicator = Ice::initialize(id);
	initialized = true;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "communicator initialized");
}

/**
 * \brief get the Communicator
 */
Ice::CommunicatorPtr	CommunicatorSingleton::get() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "communicator being retrieved");
	if (!initialized) {
		throw std::runtime_error("communicator not initialized");
	}
	return _communicator;
}

/**
 * \brief Destroy the communicator
 */
void	CommunicatorSingleton::release() {
	_communicator->destroy();
	initialized = false;
}

/**
 * \brief Static singleton object adapter
 */
Ice::ObjectAdapterPtr	CommunicatorSingleton::_adapter;
static std::mutex	mutex;

/**
 * \brief get the unique active object adapter for the communicator singleton
 */
Ice::ObjectAdapterPtr	CommunicatorSingleton::getAdapter() {
	std::unique_lock<std::mutex>	lock(mutex);
	if (_adapter) {
		return _adapter;
	}
	_adapter = CommunicatorSingleton::get()->createObjectAdapter("");
	if (!_adapter) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no adapter found");
	}
	_adapter->activate();
	return _adapter;
}

/**
 * \brief add a servant to the adapter
 */
Ice::Identity	CommunicatorSingleton::add(Ice::ObjectPtr servant) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add servant to the adapter");
	// create a new identity for the servant
	Ice::Identity	identity;
	identity.name = IceUtil::generateUUID();
	identity.category = "";

	// add the servant to the adapter
	getAdapter()->add(servant, identity);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registered %s as %s",
		astro::demangle_cstr(*servant),
		identity.name.c_str());

	// return the identity to the caller
	return identity;
}

/**
 * \brief Remove a servant from the adapter
 */
void	CommunicatorSingleton::remove(Ice::Identity identity) {
	Ice::ObjectPtr	servant = getAdapter()->remove(identity);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "servant %s with identity %s removed",
		astro::demangle_cstr(*servant),
		identity.name.c_str());
}

/**
 * \brief Connect the adapter to a connection of a proxy
 *
 * This step is required for the server to be able to send callbacks back
 * over the connection of this proxy
 */
void	CommunicatorSingleton::connect(Ice::ObjectPrx proxy) {
	if (connected) {
		debug(LOG_WARNING, DEBUG_LOG, 0, "already connected");
	}
	if (!proxy) {
		std::string	msg = astro::stringprintf("cannot connect "
			"without a proxy");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding adapter to connection of %s",
		astro::demangle_cstr(*proxy));
	auto	connection = proxy->ice_getConnection();
	if (!connection) {
		std::string	msg("no connection available");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding adapter to %s",
		astro::demangle_cstr(connection));
	connection->setAdapter(getAdapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connected");
	connected = true;
}

} // namespace snowstar
