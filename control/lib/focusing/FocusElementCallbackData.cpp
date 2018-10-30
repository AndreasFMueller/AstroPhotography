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

std::string	FocusElementCallbackData::toString() const {
	return stringprintf("pos=%d raw=%s %s evaluated=%s value=%f",
		position(), raw_image()->info().c_str(), method().c_str(),
		processed_image()->info().c_str(), value());
}

} // namespace focusing
} // namespace astro
