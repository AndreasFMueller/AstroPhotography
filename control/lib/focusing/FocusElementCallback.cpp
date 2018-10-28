/*
 * FocusElementCallback.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

using namespace astro::callback;

namespace astro {
namespace focusing {

FocusElementCallback::FocusElementCallback() {
}

FocusElementCallback::~FocusElementCallback() {
}

/**
 * \brief Get the contained data
 *
 * A NULL pointer is returned if the data is not FocusElementCallbackData
 */
FocusElementCallbackData*	FocusElementCallback::unpacked(
					CallbackDataPtr cbd) {
	FocusElementCallbackData	*u
		= dynamic_cast<FocusElementCallbackData *>(&*cbd);
	return u;
}

/**
 * \brief Send a focus element to this callback
 */
void	FocusElementCallback::send(const FocusElement& element) {
	CallbackDataPtr	cbd(new FocusCallbackData(element));
	(*this)(cbd);
}

/**
 * \brief rocess the callback data
 *
 * This callback handles only data elements of type FocusElementCallbackData,
 * all other elements are just ignored.
 */
CallbackDataPtr	FocusElementCallback::operator()(CallbackDataPtr cbd) {
	FocusElementCallbackData	*fe = unpacked(cbd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback called: %p", fe);
	if (NULL == fe) {
		return cbd;
	}
	handle(*fe);
	return cbd;
}

} // namespace focusing
} // namespace astro
