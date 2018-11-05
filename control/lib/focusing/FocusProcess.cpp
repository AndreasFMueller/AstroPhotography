/*
 * FocusProcess.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct a FocusProcess from position interval an devices
 *
 * \param minposition	first focuser position
 * \param maxposition	last focuser position
 * \param ccd		CCD device
 * \param focuser	focuser device
 */
FocusProcess::FocusProcess(unsigned long minposition,
	unsigned long maxposition, camera::CcdPtr ccd,
	camera::FocuserPtr focuser)
	: FocusProcessBase(minposition, maxposition),
	  _ccd(ccd), _focuser(focuser) {
}

/**
 * \brief Construct a FocusProcess from parameters and devices
 *
 * \param parameters	Parameter structure for the fcous process
 * \param ccd		CCD device
 * \param focuser	focuser device
 */
FocusProcess::FocusProcess(const FocusParameters& parameters,
	camera::CcdPtr ccd, camera::FocuserPtr focuser)
	: FocusProcessBase(parameters), _ccd(ccd), _focuser(focuser) {
}

/**
 * \brief Move to a position
 *
 * \param pos	focuser position to move to
 */
void	FocusProcess::moveto(unsigned long pos) {
	// backlash compensation
	long	bl = _focuser->backlash();
	if (bl != 0) {
		long	cur = _focuser->current();
		if (cur > pos) {
			long	backpos = pos - bl;
			_focuser->set(backpos);
			do {
				Timer::sleep(0.1);
			} while (_focuser->current() != backpos);
		}
	}
	// now move to the target position
	_focuser->moveto(pos);
	do {
		Timer::sleep(0.1);
	} while (_focuser->current() != pos);
}

/**
 * \brief Get an image at the current position
 */
ImagePtr	FocusProcess::get() {
	_ccd->startExposure(exposure());
	if (_ccd->wait()) {
		return _ccd->getImage();
	}
	throw std::runtime_error("cannot get image");
}

} // namespace focusing
} // namespace astro
