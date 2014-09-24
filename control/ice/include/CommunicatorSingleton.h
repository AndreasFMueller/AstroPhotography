/*
 * CommunicatorSingleton.h -- a singleton wrapper for the communicator
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CommunicatorSingleton_h
#define _CommunicatorSingleton_h

#include <Ice/Ice.h>

namespace snowstar {

class CommunicatorSingleton {
static Ice::CommunicatorPtr	_communicator;
public:
	CommunicatorSingleton(int& argc, char *argv[]);
static Ice::CommunicatorPtr	get();
};

} // namespace snowstar

#endif /* _CommunicatorSingleton_h */
