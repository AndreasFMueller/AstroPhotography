/*
 * SxGuiderPort.h -- Starlight Express guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxGuiderPort_h
#define _SxGuiderPort_h

#include <AstroCamera.h>
#include <SxCamera.h>
#include <BasicGuiderport.h>

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Starlight Express Guider Port interface
 *
 * This class encapsulates a thread that handles the timing of the guider
 * port output signals.
 */
class SxGuiderPort : public BasicGuiderport {
	SxCamera&	camera;
public:
	SxGuiderPort(SxCamera& camera);
	virtual ~SxGuiderPort();
	virtual void	do_activate(uint8_t active);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxGuiderPort_h */
