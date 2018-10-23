/*
 * FocusInputBase.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroIO.h>

namespace astro {
namespace focusing {

FocusInputBase::FocusInputBase() : _method("fwhm"), _solver("abs") {
}

ImagePtr	FocusInputBase::image(const std::string& filename) const {
	io::FITSin	in(filename);
	ImagePtr	result = in.read();
}

} // namespace focusing
} // namespace astro
