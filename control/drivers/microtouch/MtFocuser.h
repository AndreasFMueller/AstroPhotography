/*
 * MtFocuser.h -- microtouch focuser definitions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Microtouch Focuser definitions
 */
#ifndef _MtFocuser_h
#define _MtFocuser_h

#include <AstroCamera.h>
#include <MicroTouch.h>

namespace astro {
namespace device {
namespace microtouch {

class MtFocuser : public astro::camera::Focuser {
	MicroTouch	*mt;
public:
	MtFocuser();
	~MtFocuser();
	virtual long	max();
	virtual long	current();
	virtual void	set(long value);
};

} // namespace microtouch
} // namespace device
} // namespace astro

#endif /* _MtFocuser_h */
