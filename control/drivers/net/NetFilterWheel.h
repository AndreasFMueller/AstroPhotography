/*
 * NetFilterWheel.h -- network/corba based FilterWheel driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetFilterWheel_h
#define _NetFilterWheel_h

#include <AstroCamera.h>
#include "../../idl/device.hh"

namespace astro {
namespace camera {
namespace net {

class NetFilterWheel : public FilterWheel {
	FilterWheel_var	_filterwheel;
public:
	NetFilterWheel(FilterWheel_var filterwheel);
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual std::string	filterName(size_t filterindex);
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetFilterWheel_h */
