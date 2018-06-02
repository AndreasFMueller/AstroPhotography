/*
 * SxFilterWheel.h -- Filter wheel implementation for the Filter wheel
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _SxFilterWheel_h
#define _SxFilterWheel_h

#include <AstroCamera.h>
#include <AstroUSB.h>

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Starlight Express filterwheel class
 */
class SxFilterWheel : public FilterWheel {
	unsigned int	nfilters;
	std::vector<std::string>	filternames;
	astro::usb::DevicePtr	deviceptr;
public:
	SxFilterWheel(const DeviceName& name);
	~SxFilterWheel();
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual void	select(const std::string& filtername);
	virtual std::string	filterName(size_t filterindex);
	virtual FilterWheel::State	getState();
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxFilterWheel_h */
