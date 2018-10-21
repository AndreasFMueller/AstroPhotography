/*
 * FocusInputBase.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

FocusInputBase::FocusInputBase() : _method("fwhm"), _solver("abs") {
}

} // namespace focusing
} // namespace astro
