/*
 * FocusableImageConverter.cpp -- Class to extract focusable float images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <typeinfo>
#include <AstroFocus.h>
#include <AstroAdapter.h>
#include <AstroUtils.h>
#include "FocusableImageConverterImpl.h"
#include <AstroDebug.h>

namespace astro {
namespace focusing {

FocusableImageConverterPtr	FocusableImageConverter::get() {
	return FocusableImageConverterPtr(new FocusableImageConverterImpl());
}

FocusableImageConverterPtr	FocusableImageConverter::get(
					const ImageRectangle& rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get converter for rectangle %s",
		rectangle.toString().c_str());
	return FocusableImageConverterPtr(
		new FocusableImageConverterImpl(rectangle));
}

} // namespace focusing
} // namespace astro
