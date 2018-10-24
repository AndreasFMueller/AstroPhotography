/*
 * FocusCallbackData.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

FocusCallbackData::FocusCallbackData(astro::image::ImagePtr image,
	int position, double value)
	: ImageCallbackData(image), _position(position), _value(value) {
}

FocusCallbackData::FocusCallbackData(const FocusElement& fe)
	: ImageCallbackData(fe.processed_image), _position(fe.pos()),
	  _value(fe.value)  {
}

} // namespace focusing
} // namespace astro
