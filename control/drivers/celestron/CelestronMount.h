/*
 * CelestronMount.h -- interface class for Celestron mounts
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _CelestronMount_h
#define _CelestronMount_h

#include <AstroDevice.h>
#include <Serial.h>

namespace astro {
namespace driver {
namespace celestron {

class CelestronMount : public astro::device::Mount,
			public astro::device::Serial {
	void	getprompt();
	double	angle(uint16_t a);
	double	angle(uint32_t a);
	uint16_t	angle16(const Angle& a);
	uint32_t	angle32(const Angle& a);
	std::pair<double, double>	parseangles(const std::string& s);
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

#endif /* _CelestronMount */
