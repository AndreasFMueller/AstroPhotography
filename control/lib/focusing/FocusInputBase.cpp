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

FocusInputBase::FocusInputBase(const std::string& method,
	const std::string& solver) : _method(method), _solver(solver) {
}

ImagePtr	FocusInputBase::image(const std::string& filename) const {
	io::FITSin	in(filename);
	ImagePtr	result = in.read();
	return result;
}

} // namespace focusing
} // namespace astro
