/*
 * AtikCcd.h -- declaration of Atik CCD class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikCcd_h
#define _AtikCcd_h

#include <AstroCamera.h>
#include <atikccdusb.h>
#include <thread>

namespace astro {
namespace camera {
namespace atik {

class AtikCcd : public Ccd {
	::AtikCamera	*_camera;
	struct AtikCapabilities	capa;
	ImagePtr	_image;
	std::shared_ptr<std::thread>	_thread;
public:
	AtikCcd(CcdInfo&, ::AtikCamera *);
	~AtikCcd();
	virtual void	startExposure(const Exposure& exposure);
	virtual void	cancelExposure();

private:
	ImagePtr	getRawImage();
	bool	hasShutter() const;

protected:
	CoolerPtr	getCooler0();
public:
	bool	hasCooler() const;

public:
	void	run();
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikCcd_h */
