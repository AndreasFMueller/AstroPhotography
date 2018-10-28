/*
 * FocusElementCallbackData.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

FocusElementCallbackData::FocusElementCallbackData(const FocusElement& e)
	: _position(e.pos()), _raw_image(e.raw_image), _method(e.method),
	  _processed_image(e.processed_image), _value(e.value) {
}

} // namespace focusing
} // namespace astro
