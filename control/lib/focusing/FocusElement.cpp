/*
 * FocusElement.cpp -- FocusElement implementation
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroIO.h>

#include <sstream>

namespace astro {
namespace focusing {

/**
 * \brief Construct a focus element
 */
FocusElement::FocusElement(unsigned long pos) : _pos(pos) {
	value = 0;
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

/**
 * \brief Get a string representation
 */
std::string	FocusElement::toString() const {
	std::ostringstream	out;
	out << "position=" << pos();
	if (filename.size() > 0) {
		out << ", filename=" << filename;
	}
	if (raw_image) {
		out << ", raw image=" << raw_image->info();
	}
	if (processed_image) {
		out << ", processed image=" << processed_image->info();
	}
	if (value > 0) {
		out << ", value=" << value;
	}
	return out.str();
}

} // namespace focusing
} // namespace astro
