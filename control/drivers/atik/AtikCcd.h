/*
 * AtikCcd.h -- declaration of Atik CCD class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikCcd_h
#define _AtikCcd_h

#include <AtikCamera.h>
#include <atikccdusb.h>
#include <thread>

namespace astro {
namespace camera {
namespace atik {

class AtikCcd;

class AtikCcd : public Ccd {
	astro::camera::atik::AtikCamera&	_camera;
	ImagePtr	_image;
public:
	void	image(ImagePtr);
private:
	std::shared_ptr<std::thread>	_thread;
public:
	AtikCcd(CcdInfo&, astro::camera::atik::AtikCamera&);
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

	void	updatestate(CcdState::State s) { state(s); }
	virtual std::string	userFriendlyName() const {
		return _camera.userFriendlyName();
	}
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikCcd_h */
