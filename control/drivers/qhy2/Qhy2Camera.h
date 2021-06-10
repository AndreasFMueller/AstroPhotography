/*
 * Qhy2Camera.h -- QHY camera interface
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Qhy2Camera_h
#define _Qhy2Camera_h

#include <AstroCamera.h>
#include <AstroUSB.h>
#include <qhyccd.h>

namespace astro {
namespace camera {
namespace qhy2 {

class Qhy2Ccd;
class Qhy2Cooler;
class Qhy2GuidePort;

/**
 * \brief QHY Camera class
 *
 * This is mainly a wrapper class that is used to forward commands to
 * the device class from the QHY library
 */
class Qhy2Camera : public astro::camera::Camera {
	qhyccd_handle	*_handle;
	qhyccd_handle	*handle() { return _handle; }
public:
	Qhy2Camera(const std::string& qhyname);
	virtual ~Qhy2Camera();
	std::string	qhyname() const;
protected:
	virtual CcdPtr	getCcd0(size_t id);
	virtual GuidePortPtr	getGuidePort0();
	friend class Qhy2Ccd;
	friend class Qhy2Cooler;
	friend class Qhy2GuidePort;
private:
	std::vector<std::string>	_readoutmode_names;
public:
	std::string	readoutmode(uint32_t mode) const;
	uint32_t	readoutmode(const std::string& mode) const;
	CcdInfo		getinfo(uint32_t mode, int bits, int ccdindex);
	uint32_t	readoutmode(const CcdInfo& info) const;
};

} // namespace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2Camera_h */
