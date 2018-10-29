/*
 * FocusingCallback.cpp -- focusing servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusingI.h>
#include <IceConversions.h>
#include <Ice/Connection.h>
#include <ProxyCreator.h>
#include <AstroDebug.h>

namespace snowstar {

//////////////////////////////////////////////////////////////////////
// Focusing callback adapter
//////////////////////////////////////////////////////////////////////
template<>
void	callback_adapter<FocusCallbackPrx>(FocusCallbackPrx& prx,
		const astro::callback::CallbackDataPtr data) {
	// Handle FocusElementCallbackData
	astro::focusing::FocusElementCallbackData	*fedata
		= dynamic_cast<astro::focusing::FocusElementCallbackData *>(&*data);
	if (NULL != fedata) {
		FocusElementPtr	feptr = convert(*fedata,
						astro::image::Format::PNG);
		prx->addFocusElement(*feptr);
	}

	// Handle FocusCallbackData
	astro::focusing::FocusCallbackData	*focusdata
		= dynamic_cast<astro::focusing::FocusCallbackData *>(&*data);
	if (NULL != focusdata) {
		FocusPoint	p;
		p.position = focusdata->position();
		p.value = focusdata->value();
		prx->addPoint(p);
	}

	// Handle FocusCallbackState
	astro::focusing::FocusCallbackState	*focusstate
		= dynamic_cast<astro::focusing::FocusCallbackState *>(&*data);
	if (NULL != focusstate) {
		prx->changeState(convert(focusstate->state()));
	}
}

//////////////////////////////////////////////////////////////////////
// focusing callback
//////////////////////////////////////////////////////////////////////
astro::callback::CallbackDataPtr	FocusingCallback::operator()(
	astro::callback::CallbackDataPtr data) {
	_focusing.updateFocusing(data);
	return data;
}

} // namespace snowstar
