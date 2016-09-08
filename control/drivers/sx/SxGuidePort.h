/*
 * SxGuidePort.h -- Starlight Express guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxGuidePort_h
#define _SxGuidePort_h

#include <AstroCamera.h>
#include "SxCamera.h"
#include <BasicGuideport.h>

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Starlight Express Guider Port interface
 *
 * This class encapsulates a thread that handles the timing of the guider
 * port output signals.
 */
class SxGuidePort : public BasicGuideport {
	SxCamera&	camera;
	SxGuidePort(const SxGuidePort&);
	SxGuidePort&	operator=(const SxGuidePort&);
public:
	SxGuidePort(SxCamera& camera);
	virtual ~SxGuidePort();
	virtual void	do_activate(uint8_t active);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxGuidePort_h */
