/*
 * CelestronMount.h -- interface class for Celestron mounts
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <AstroDevice.h>
#include <Serial.h>

namespace astro {
namespace driver {
namespace celestron {

class CelestronMount : public astro::device::Device, public astro::device::Serial {
public:
	CelestronMount(const std::string& devicename);
	virtual ~CelestronMount();

	// accessors
	virtual astro::device::Mount::mount_state	state();
	virtual RaDec	getRaDec();
	virtual AzmAlt	getAzmAlt();
	virtual void	Goto(const RaDec& radec);
	virtual void	Goto(const AzmAlt& azmalt);
	virtual void	cancel();
};

} // namepace celestron
} // namespace driver
} // namespace astro
