/*
 * Qhy2Ccd.h -- QHY device
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Qhy2Ccd_h
#define _Qhy2Ccd_h

#include <AstroCamera.h>
#include <Qhy2Camera.h>
#include <qhyccd.h>
#include <Qhy2Utils.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief QHY ccd class
 */
class Qhy2Ccd : public Ccd {
	bool	_hasgain;
	float	_gain;
	std::pair<float,float>	_gaininterval;
public:
	bool	hasGain() { return _hasgain; }
	float	getGain() { return _gain; }
	std::pair<float,float>	gainInterval() { return _gaininterval; }
private:
	uint32_t	_readoutmode;
	int	_bits;
public:
	uint32_t	readoutmode() const { return _readoutmode; }
	void	readoutmode(uint32_t r) { _readoutmode = r; }
private:
	std::thread	thread;
	ImagePtr	image;
	Qhy2Camera&	camera;
	double	getExposuretime(float exposuretime);
public:
	Qhy2Ccd(const CcdInfo& info, Qhy2Camera& _camera);
	virtual ~Qhy2Ccd();
public:
	virtual void	startExposure(const Exposure& exposure);
private:
	static void	main(Qhy2Ccd *ccd) noexcept;
public:
	virtual void	getImage0();
	virtual ImagePtr	getRawImage();
	virtual bool	hasCooler() const { return true; }
protected:
	virtual CoolerPtr	getCooler0();
};

} // namespace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2Ccd_h */
