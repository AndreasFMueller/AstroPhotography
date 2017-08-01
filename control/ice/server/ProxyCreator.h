/*
 * ProxyCreator.h -- template to simplify proxy creation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ProxyCreator_h
#define _ProxyCreator_h

#include <string>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <NameConverter.h>
#include <AstroDebug.h>

namespace snowstar {

template<typename prx>
prx createProxy(const std::string& name, const Ice::Current& current,
		bool encoded = true) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create proxy named %s", name.c_str());
	Ice::ObjectAdapterPtr	a = current.adapter;
	Ice::CommunicatorPtr	ic = a->getCommunicator();
	std::string	ename = name;
	if (encoded) {
		ename = NameConverter::urlencode(name);
	}
	return prx::uncheckedCast(a->createProxy(ic->stringToIdentity(ename)));
}

} // namespace snowstar

#endif /* _ProxyCreator_h */
