/*
 * GuiderCalibrationRedirector.h -- Callback to redirect calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderCalibrationRedirector_h
#define _GuiderCalibrationRedirector_h

#include <AstroGuiding.h>
#include <AstroCallback.h>

using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief Auxiliary class to ensure calibrations found are sent to the guider
 */
class GuiderCalibrationRedirector : public Callback {
	Guider	*_guider;
public:
	GuiderCalibrationRedirector(Guider *guider) : _guider(guider) { }
	CallbackDataPtr	operator()(CallbackDataPtr data);
};

} // namespace guiding
} // namespace astro

#endif /* _GuiderCalibrationRedirector_h */
