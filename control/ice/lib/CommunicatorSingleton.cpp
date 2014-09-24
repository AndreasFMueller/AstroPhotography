/*
 * CommunicatorSingleton.cpp -- 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>

namespace snowstar {

static bool	initialized = false;
Ice::CommunicatorPtr	CommunicatorSingleton::_communicator;

CommunicatorSingleton::CommunicatorSingleton(int& argc, char *argv[]) {
	_communicator = Ice::initialize(argc, argv);
	initialized = true;
}

Ice::CommunicatorPtr	CommunicatorSingleton::get() {
	if (!initialized) {
		throw std::runtime_error("communicator not initialized");
	}
	return _communicator;
}

} // namespace snowstar
