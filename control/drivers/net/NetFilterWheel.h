/*
 * NetFilterWheel.h -- network/corba based FilterWheel driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetFilterWheel_h
#define _NetFilterWheel_h

#include <AstroCamera.h>
#include <device.hh>

namespace astro {
namespace camera {
namespace net {

/**
 * \brief Network client for a filterwheel
 */
class NetFilterWheel : public FilterWheel {
	Astro::FilterWheel_var	_filterwheel;
public:
	NetFilterWheel(Astro::FilterWheel_var filterwheel);
	~NetFilterWheel();
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual std::string	filterName(size_t filterindex);
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetFilterWheel_h */
