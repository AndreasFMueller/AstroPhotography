/*
 * AtikFilterwheel.h -- declaration of ATIK filterwheel class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikFilterwheel_h
#define _AtikFilterwheel_h

#include <atikccdusb.h>
#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikFilterwheel : public FilterWheel {
	::AtikCamera	*_camera;
public:
	AtikFilterwheel(::AtikCamera *);
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual State	getState();
	virtual std::string	userFriendlyName() const;
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikFilterwheel_h */
