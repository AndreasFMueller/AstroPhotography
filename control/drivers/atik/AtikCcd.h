/*
 * AtikCcd.h -- declaration of Atik CCD class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikCcd_h
#define _AtikCcd_h

#include <AstroCamera.h>
#include <atikccdusb.h>

namespace astro {
namespace camera {
namespace atik {

class AtikCcd : public Ccd {
	::AtikCamera	*_camera;
public:
	AtikCcd(CcdInfo&, ::AtikCamera *);
	~AtikCcd();
	bool	hasCooler();
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikCcd_h */
