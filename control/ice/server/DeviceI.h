/*
 * DeviceI.h -- device servant declaration
 *
 * (c) 2015 prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceI_h
#define _DeviceI_h

#include <types.h>
#include <AstroDevice.h>
#include <StatisticsI.h>

namespace snowstar {

class DeviceI : virtual public Device, public StatisticsI {
	astro::device::Device&	_device;
public:
	DeviceI(astro::device::Device& device);
	virtual ~DeviceI();
	virtual std::string	getName(const Ice::Current& current);
	virtual stringlist	parameterNames(const Ice::Current& current);
	virtual bool	hasParameter(const std::string& name,
		const Ice::Current& current);
	virtual ParameterDescription	parameter(const std::string& name,
		const Ice::Current& current);
	virtual void	setParameterFloat(const std::string& name, double value,
		const Ice::Current& current);
	virtual void	setParameterString(const std::string& name,
		const std::string& value, const Ice::Current& current);
	virtual float	parameterValueFloat(const std::string& name,
		const Ice::Current& current);
	virtual std::string	parameterValueString(const std::string& name,
		const Ice::Current& current);
};

} // namespace snowstar

#endif /* _DeviceI_h */
