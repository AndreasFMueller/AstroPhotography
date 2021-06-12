/*
 * CoolerInfo.cpp -- information about the current state of the cooler
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroFormat.h>

namespace astro {
namespace camera {

const Temperature&	CoolerInfo::actualTemperature() const {
	return _actualTemperature;
}
const Temperature&      CoolerInfo::setTemperature() const {
	return _setTemperature;
}
bool    CoolerInfo::on() const {
	return _on;
}

CoolerInfo::CoolerInfo(const Temperature& actualTemperature,
	const Temperature& setTemperature,  bool on)
	: _actualTemperature(actualTemperature),
	  _setTemperature(setTemperature), _on(on) {
}

CoolerInfo::CoolerInfo(Cooler& cooler)
	: _actualTemperature(cooler.getActualTemperature()),
	  _setTemperature(cooler.getSetTemperature()),
	  _on(cooler.isOn()) {
}

std::string	CoolerInfo::toString() const {
	return stringprintf("actual=%.1f, set=%.1f, on=%s",
		_actualTemperature.celsius(),
		_setTemperature.celsius(),
		(_on) ? "yes" : "no");
}

} // namespace camera
} // namespace astro
