/*
 * DeviceNicer.cpp -- Device nice class implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Nice.h>

namespace astro {
namespace device {
namespace nice {

DeviceNicer::DeviceNicer(const std::string& servicename)
	: _servicename(servicename) {
}

DeviceName	DeviceNicer::operator()(const DeviceName& original) {
	std::vector<std::string>	components;
	components.push_back("nice");
	components.push_back(_servicename);
	copy(original.begin(), original.end(), back_inserter(components));
	DeviceName	result(original.type(), components);
	return result;
}

std::string	DeviceNicer::operator()(const std::string& name) {
	DeviceName	original(name);
	return (*this)(original);
}

std::vector<std::string>	DeviceNicer::operator()(const std::vector<std::string>& names) {
	std::vector<std::string>	result;
	std::vector<std::string>::const_iterator	i;
	for (i = names.begin(); i != names.end(); i++) {
		result.push_back((*this)(*i));
	}
	return result;
}

} // namespace nice
} // namespace device
} // namespace astro
