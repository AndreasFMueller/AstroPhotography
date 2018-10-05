/*
 * CommunicatorSingleton.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <IceUtil/UUID.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <stdexcept>
#include <mutex>

namespace snowstar {

static bool	initialized = false;
Ice::CommunicatorPtr	CommunicatorSingleton::_communicator;

/**
 * \brief Create the Communicator singleton
 */
CommunicatorSingleton::CommunicatorSingleton(int& argc, char *argv[]) {
	// extract properties from the command line
	Ice::PropertiesPtr props = Ice::createProperties(argc, argv);

	// set property to turn off ACM, because it will be useless in
	// all the programs that use fixed proxies and callbacks
	// XXX temporarily turned off
	//props->setProperty("Ice.ACM.Client", "0");

	// the large message size is required because we have cases
	// where we transfer entire images as messages
	props->setProperty("Ice.MessageSizeMax", "65536");

	// abort on null handle errors
	props->setProperty("Ice.NullHandleAbort", "1");

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
	_adapter->activate();
	return _adapter;
}

/**
 * \brief add a servant to the adapter
 */
Ice::Identity	CommunicatorSingleton::add(Ice::ObjectPtr servant) {
	// create a new identity for the servant
	Ice::Identity	identity;
	identity.name = IceUtil::generateUUID();
	identity.category = "";

	// add the servant to the adapter
	getAdapter()->add(servant, identity);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registered %s as %s",
		astro::demangle(typeid(*servant).name()).c_str(),
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
		astro::demangle(typeid(*servant).name()).c_str(),
		identity.name.c_str());
}

/**
 * \brief Connect the adapter to a connection of a proxy
 *
 * This step is required for the server to be able to send callbacks back
 * over the connection of this proxy
 */
void	CommunicatorSingleton::connect(Ice::ObjectPrx proxy) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding adapter to connection of %s",
		astro::demangle(typeid(*proxy).name()).c_str());
	proxy->ice_getConnection()->setAdapter(getAdapter());
}

} // namespace snowstar
