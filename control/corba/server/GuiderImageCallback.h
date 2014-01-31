/*
 * GuiderImageCallback.h -- Callback that sends images to the monitor
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <AstroCallback.h>

namespace Astro {

class GuiderImageCallback : public astro::callback::Callback {
	Guider_impl&	_guider;
public:
	GuiderImageCallback(Guider_impl& guider);
	virtual astro::callback::CallbackDataPtr operator()(
		astro::callback::CallbackDataPtr data);
};

} // namespace Astro
