/*
 * AtikGuideport.h -- class to control guide ports of ATIK cameras
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikGuideport_h
#define _AtikGuideport_h

#include <atikccdusb.h>
#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikGuideport : public GuidePort {
	::AtikCamera	*_camera;
public:
	AtikGuideport(::AtikCamera*);
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikGuideport_h */
