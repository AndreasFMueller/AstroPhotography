/*
 * Qhy2Utils.cpp -- utility functions for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Qhy2Utils.h>
#include <Qhy2Locator.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <qhyccd.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief Translate an error code to an error message
 *
 * \int err 	error code used by the QHYCDD API
 */
std::string	Qhy2Error::err2string(int err) {
	return stringprintf("error %d", err);
}

/**
 * \brief auxiliary function to produce the module base name
 */
std::vector<std::string>	Qhy2Name::qhybasename() {
	std::vector<std::string>	result;
	result.push_back("qhy2");
	return result;
}

/**
 * \brief Construct a QHY name basd on the device name
 *
 * \param qhyindex	the index of the camera in the QHYCCD api
 */
Qhy2Name::Qhy2Name(int qhyindex)
	: DeviceName(DeviceName::Camera, qhybasename()),
	  _qhyindex(qhyindex) {
	char	camId[32];
	int	rc = GetQHYCCDId(_qhyindex, camId);
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf(
			"cannot get qhyname for %d (%d)", qhyindex, rc);
		throw Qhy2Error(msg, rc);
	}
	_qhyname = std::string(camId);
	push_back(_qhyname);
}

Qhy2Name::Qhy2Name(const std::string& qhyname) 
	: DeviceName(DeviceName::Camera, qhybasename()) {
	_qhyindex = -1;
	_qhyname = qhyname;
	push_back(_qhyname);
}

/**
 * \brief Construct a camera name
 */
DeviceName	Qhy2Name::cameraname() const {
	DeviceName	result = *this;
	result.type(DeviceName::Camera);
	return result;
}

/**
 * \brief Construct a cooler name
 */
DeviceName	Qhy2Name::coolername() const {
	return DeviceName(*this, DeviceName::Cooler);
}

/**
 * \brief Construct a filterwheel name
 */
DeviceName	Qhy2Name::filterwheelname() const {
	return DeviceName(*this, DeviceName::Filterwheel);
}

/**
 * \brief Construct a guideport name
 */
DeviceName	Qhy2Name::guideportname() const {
	return DeviceName(*this, DeviceName::Guideport);
}

/**
 * \brief Construct a ccd name
 *
 * \param ccd	the name of the ccd
 */
DeviceName	Qhy2Name::ccdname(const std::string& ccd) const {
	return DeviceName(*this, DeviceName::Ccd, ccd);
}

} // namespace qhy2
} // namespace camera
} // namespace astro
