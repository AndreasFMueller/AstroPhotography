/*
 * CalibrationRedirector.h -- Callback to redirect calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CalibrationRedirector_h
#define _CalibrationRedirector_h

#include <AstroGuiding.h>
#include <AstroCallback.h>

using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief Auxiliary class to ensure calibrations found are sent to the guider
 */
class CalibrationRedirector : public Callback {
	Guider	*_guider;
public:
	CalibrationRedirector(Guider *guider) : _guider(guider) { }
	CallbackDataPtr	operator()(CallbackDataPtr data);
};

} // namespace guiding
} // namespace astro

#endif /* _CalibrationRedirector_h */
