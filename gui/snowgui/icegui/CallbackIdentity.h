/*
 * CallbackIdentity.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _CallbackIdentity_h
#define _CallbackIdentity_h

#include <Ice/Ice.h>

namespace snowgui {

class CallbackIdentity {
	Ice::Identity	_identity;
public:
	CallbackIdentity() noexcept;
	const Ice::Identity& identity() const { return _identity; }
	static Ice::Identity identity(Ice::ObjectPtr object);
};

} // namespace snowgui

#endif /* _CallbackIdentity_h */
