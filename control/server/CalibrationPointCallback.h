/*
 * CalibrationPointCallback.h -- callback for calibration points
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <AstroGuiding.h>
#include <AstroCallback.h>
#include <CalibrationPersistence.h>
#include <Conversions.h>

namespace Astro {

class CalibrationPointCallback : public astro::callback::Callback {
	Guider_impl&	_guider;
	long	_calibrationid;
public:
	long	calibrationid() const { return _calibrationid; }
public:
	CalibrationPointCallback(Guider_impl& guider);
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data);
};

} // namespace Astro
