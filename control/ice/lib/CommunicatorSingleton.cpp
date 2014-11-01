/*
 * CommunicatorSingleton.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <AstroDebug.h>
#include <stdexcept>

namespace snowstar {

static bool	initialized = false;
Ice::CommunicatorPtr	CommunicatorSingleton::_communicator;

/**
 * \brief Create the Communicator singleton
 */
CommunicatorSingleton::CommunicatorSingleton(int& argc, char *argv[]) {
	// set property to turn of ACM, because it will be useless in
	// all the programs that use fixed proxies and callbacks
	Ice::PropertiesPtr props = Ice::createProperties(argc, argv);
	props->setProperty("Ice.ACM.Client", "0");
	props->setProperty("Ice.MessageSizeMax", "65536");
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

} // namespace snowstar
