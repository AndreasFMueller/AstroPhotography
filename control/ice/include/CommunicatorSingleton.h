/*
 * CommunicatorSingleton.h -- a singleton wrapper for the communicator
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CommunicatorSingleton_h
#define _CommunicatorSingleton_h

#include <Ice/Ice.h>

namespace snowstar {
/**
 * \brief ICE related datastructures that appear only once
 */
class CommunicatorSingleton {
	static Ice::CommunicatorPtr	_communicator;
	static Ice::ObjectAdapterPtr	_adapter;
public:
	// constructor: create at the beginning of the program, so that
	// command line parameters can be evaluated by the ICE library
	CommunicatorSingleton(int& argc, char *argv[]);

	// get the communicator
	static Ice::CommunicatorPtr	get();
	static void	release();

	// maintaining the adapter and adding/removing servants 
	static Ice::ObjectAdapterPtr	getAdapter();
	static Ice::Identity	add(Ice::ObjectPtr servant);
	static void	remove(Ice::Identity servantidentity);

	// adding the adapter to a proxy's connection
	static void	connect(Ice::ObjectPrx proxy);
};

} // namespace snowstar

#endif /* _CommunicatorSingleton_h */
