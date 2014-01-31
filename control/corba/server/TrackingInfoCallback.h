/*
 * TrackingInfoCallback.h -- Callback for tracking info
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <AstroCallback.h>

namespace Astro {

/**
 * \brief a callback to to record tracking info
 */
class TrackingInfoCallback : public astro::callback::Callback {
	Guider_impl&	_guider;
	long	_guidingrunid;
public:
	long	guidingrunid() const { return _guidingrunid; }
public:
	TrackingInfoCallback(Guider_impl& guider);
	virtual	astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data);
};

} // namespace Astro
