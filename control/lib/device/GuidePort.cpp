/* 
 * GuidePort.cpp -- Guider Port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroCallback.h>

namespace astro {
namespace camera {

DeviceName::device_type	GuidePort::devicetype = DeviceName::Guideport;

DeviceName	GuidePort::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Guideport, unitname);
}

GuidePort::GuidePort(const DeviceName& name)
	: Device(name, DeviceName::Guideport) {
}

GuidePort::GuidePort(const std::string& name)
	: Device(name, DeviceName::Guideport) {
}

GuidePort::~GuidePort() {
}

void	GuidePort::activate(const GuidePortActivation& a) {
	callback(a);
	this->activate(a.raplus(), a.raminus(), a.decplus(), a.decminus());
}

void	GuidePort::addCallback(callback::CallbackPtr callback) {
	_callback.insert(callback);
}

void	GuidePort::removeCallback(callback::CallbackPtr callback) {
	auto	i = _callback.find(callback);
	if (i != _callback.end()) {
		_callback.erase(i);
	}
}

void	GuidePort::callback(const GuidePortActivation& a) {
	callback::CallbackDataPtr	data(new ActivationCallbackData(a));
	_callback(data);
}

} // namespace camera
} // namespace astro
