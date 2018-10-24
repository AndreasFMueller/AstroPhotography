/*
 * FocusProcess.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

FocusProcess::FocusProcess(unsigned long minposition,
	unsigned long maxposition, camera::CcdPtr ccd,
	camera::FocuserPtr focuser)
	: FocusProcessBase(minposition, maxposition),
	  _ccd(ccd), _focuser(focuser) {
}

void	FocusProcess::moveto(unsigned long) {
	// XXX implementation missing
}

ImagePtr	FocusProcess::get() {
	// XXX implementation missing
	ImagePtr	image;
	return image;
}

} // namespace focusing
} // namespace astro
