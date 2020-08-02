/*
 * CcdICallback.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <CcdI.h>
#include <AstroDebug.h>

namespace snowstar {

CcdICallback::CcdICallback(CcdI& ccd) : _ccd(ccd) {
}

astro::callback::CallbackDataPtr        CcdICallback::operator()(
	astro::callback::CallbackDataPtr data) {
	astro::camera::CcdStateCallbackData *cs
		= dynamic_cast<astro::camera::CcdStateCallbackData*>(&*data);
	if (cs != NULL) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd state callback called: %s",
			astro::camera::CcdState::state2string(cs->data()).c_str());
		
	}
	_ccd.stateUpdate(data);
	return data;
}

} // namespace snowstar
