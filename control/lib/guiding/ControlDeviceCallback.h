/*
 * ControlDeviceCallback.h -- Callback for control devices
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ControlDeviceCallback_h
#define _ControlDeviceCallback_h

#include "BasicProcess.h"

using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief Callback classes for control devices
 */
class ControlDeviceCallback : public Callback {
	ControlDeviceBase	*_controldevice;
public:
	ControlDeviceCallback(ControlDeviceBase *controldevice)
		: _controldevice(controldevice) {
	}
	CallbackDataPtr	operator()(CallbackDataPtr data);
};

} // namespace guiding
} // namespace astro

#endif /* _ControlDeviceCallback_h */
