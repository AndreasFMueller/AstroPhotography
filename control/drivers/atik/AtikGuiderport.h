/*
 * AtikGuiderport.h -- class to control guide ports of ATIK cameras
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikGuiderport_h
#define _AtikGuiderport_h

#include <atikccdusb.h>
#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikGuiderport : public GuiderPort {
	::AtikCamera	*_camera;
public:
	AtikGuiderport(::AtikCamera*);
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikGuiderport_h */
