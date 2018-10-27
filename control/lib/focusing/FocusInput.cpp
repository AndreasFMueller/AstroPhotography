/*
 * FocusInput.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <sstream>
#include <AstroIO.h>

namespace astro {
namespace focusing {

FocusInput::FocusInput() {
}

/**
 * \brief Convert the FocusInput to a string
 */
std::string     FocusInput::toString() const {
	std::ostringstream      out;
	out << "Method:      " << method() << std::endl;
	out << "Solver:      " << solver() << std::endl;
	if (rectangle() != image::ImageRectangle()) {
		out << "Rectangle:   " << rectangle().toString();
		out << std::endl;
	}
	std::for_each(begin(), end(),
		[&](const std::pair<unsigned long, std::string>& i) {
			out << i.first << " " << i.second;
			out << std::endl;
		}
	);
	return out.str();
}

/**
 * \brief Retrieve an image from a position
 *
 * \param pos	the position for which the image should be retrieved
 */
ImagePtr	FocusInput::image(unsigned long pos) const {
	auto	i = find(pos);
	if (i == end()) {
		throw std::runtime_error("no such position");
	}
	std::string	filename = i->second;
	return FocusInputBase::image(i->second);
}

} // namespace focusing
} // namespace astro
