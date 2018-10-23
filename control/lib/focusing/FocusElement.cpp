/*
 * FocusElement.cpp -- FocusElement implementation
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroIO.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct a focus element
 */
FocusElement::FocusElement(unsigned long pos) : _pos(pos) {
}

/**
 * \brief Get the image, reading it from a file if necessary
 */
ImagePtr	FocusElement::image() const {
	if (raw_image) {
		return raw_image;
	}
	io::FITSin	in(filename);
	return in.read();
}

} // namespace focusing
} // namespace astro
