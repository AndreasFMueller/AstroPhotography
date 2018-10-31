/*
 * FocusElementCallbackData.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <sstream>

namespace astro {
namespace focusing {

FocusElementCallbackData::FocusElementCallbackData(const FocusElement& e)
	: _position(e.pos()), _raw_image(e.raw_image), _method(e.method),
	  _processed_image(e.processed_image), _value(e.value) {
}

std::string	FocusElementCallbackData::toString() const {
	std::ostringstream	str;
	str << stringprintf("pos=%d raw=", position());
	if (raw_image()) {
		str << raw_image()->info();
	} else {
		str << "(nil)";
	}
	str << " method=" << method() << " evaluated=";
	if (processed_image()) {
		str << processed_image()->info();
	} else {
		str << "(nil)";
	}
	str << " value=" << value();
	return str.str();
}

} // namespace focusing
} // namespace astro
