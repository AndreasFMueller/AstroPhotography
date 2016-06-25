/*
 * AsiCcd.h --
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AsiCcd_h
#define _AsiCcd_h

#include <AstroCamera.h>
#include <AsiCamera.hh>

namespace astro {
namespace camera {
namespace asi {

class AsiCcd : public Ccd {
	AsiCamera&	_camera;
	bool	_hasCooler;
public:
	static std::string	imgtype2string(int imgtype);
	static int	string2imgtype(const std::string& name);
public:
	AsiCcd(const CcdInfo&, AsiCamera& camera);
	virtual ~AsiCcd();
	virtual void	startExposure(const Exposure& exposure);
	virtual CcdState::State	exposureStatus();
	virtual void	cancelExposure();

	// image retrieval
	virtual astro::image::ImagePtr	getRawImage();

protected:
	virtual CoolerPtr	getCooler0();
public:
	void	hasCooler(bool hc) { _hasCooler = hc; }
	virtual bool	hasCooler() const { return _hasCooler; }
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiCcd_h */
