/*
 * CallbackIdentity.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <CallbackIdentity.h>
#include <IceUtil/UUID.h>

namespace snowgui {

CallbackIdentity::CallbackIdentity() noexcept {
	_identity.name = IceUtil::generateUUID();
}

Ice::Identity	CallbackIdentity::identity(Ice::ObjectPtr object) {
	CallbackIdentity	*ci = dynamic_cast<CallbackIdentity*>(&*object);
	if (ci) {
		return ci->identity();
	}
	Ice::Identity	result;
	result.name = IceUtil::generateUUID();
	return result;
}

} // namespace snowgui
