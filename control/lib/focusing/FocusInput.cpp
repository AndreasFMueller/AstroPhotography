/*
 * FocusInput.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <sstream>

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
		[&out](const std::pair<unsigned long, std::string>& i)
			mutable {
			out << i.first << " " << i.second;
			out << std::endl;
		}
	);
	return out.str();
}

} // namespace focusing
} // namespace astro
