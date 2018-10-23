/*
 * BasicFocusProcess.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

BasicFocusProcess::BasicFocusProcess(camera::CcdPtr ccd,
	camera::FocuserPtr focuser) : _ccd(ccd), _focuser(focuser) {
}

void	BasicFocusProcess::moveto(unsigned long) {
	// XXX implementation missing
}

ImagePtr	BasicFocusProcess::get(const camera::Exposure& exposure) {
	// XXX implementation missing
}


} // namespace focusing
} // namespace astro
