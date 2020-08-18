/*
 * AtikCamera.h -- Atik camera class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikCamera_h
#define _AtikCamera_h

#include <atikccdusb.h>
#include <AstroCamera.h>
#include <mutex>
#include <atomic>

namespace astro {
namespace camera {
namespace atik {

class AtikCcd;
class AtikCooler;

class AtikCamera : public Camera {
	// Atik camera structure and mutex to protect access to it
	::AtikCamera	*_camera;
	std::recursive_mutex	_mutex;
	
	struct AtikCapabilities	_capa;
	CAMERA_TYPE	_type;
	std::string	_atikname;
	unsigned int	_serial;
public:
	const std::string	atikname() const { return _atikname; }
	const struct AtikCapabilities&	capa() const { return _capa; }
	CAMERA_TYPE	type() { return _type; }
	unsigned int	getSerialNumber() const { return _serial; }
	virtual std::string	userFriendlyName() const;
private:
	ImagePtr	getImage(Exposure& exopsure);
	ImagePtr	shortExposure(const ImagePoint& offset,
				Exposure& exposure);
	ImagePtr	longExposure(const ImagePoint& offset,
				Exposure& exposure);
	ImagePtr	multiExposure(const ImagePoint& offset,
				Exposure& exposure);
	void	exposureRun(Exposure& exposure, AtikCcd& atikccd);
	void	abortExposure();
	std::string	getLastError();
public:
	AtikCamera(::AtikCamera *camera);
	virtual ~AtikCamera();
	CcdPtr	getCcd0(size_t ccdid);
	unsigned int	nCcds() const;

	// cooler related stuff
private:
	CoolerPtr	_cooler;
protected:
	CoolerPtr	getCooler0();
private:
	int	_tempSensorCount;
	std::atomic_bool	_is_on;
	std::atomic<float>	_last_actual_temperature;
	Temperature	getSetTemperature(AtikCooler& cooler);
	Temperature	getActualTemperature(AtikCooler& cooler);
	void	setTemperature(const float temperature, AtikCooler& cooler);
	bool	isOn(AtikCooler& cooler);
	void	setOn(bool onoff, AtikCooler& cooler);
	void	initiateWarmUp();

private:
	FilterWheelPtr	_filterwheel;
protected:
	virtual FilterWheelPtr	getFilterWheel0();
public:
	bool	hasFilterWheel() const;
private:
	void	getFilterWheelStatus(unsigned int *filtercount, bool *moving,
			unsigned int *current, unsigned int *target);
	void	setFilter(unsigned int filterindex);

protected:
	virtual GuidePortPtr	getGuidePort0();
public:
	bool	hasGuidePort() const;

friend class AtikCcd;
friend class AtikCooler;
friend class AtikFilterwheel;
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikCamera_h */
