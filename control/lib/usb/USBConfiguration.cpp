/*
 * USBConfiguration.cpp -- configuration implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <sstream>

namespace astro {
namespace usb {

/////////////////////////////////////////////////////////////////////
// Configuration Descriptors
/////////////////////////////////////////////////////////////////////

void	Configuration::copy(const libusb_config_descriptor *_config) {
	config = new libusb_config_descriptor;
	assert(config != NULL);
	memcpy(config, _config, sizeof(libusb_config_descriptor));
	config->extra = NULL;
	config->extra_length = 0;
}

Configuration::Configuration(Device& device,
	const struct libusb_config_descriptor *config)
	: Descriptor(device, config->extra, config->extra_length) {
	copy(config);
	getInterfaces();
}

Configuration::~Configuration() {
	delete config;
}

uint8_t Configuration::bConfigurationValue() const {
	return config->bConfigurationValue;
}

uint8_t Configuration::bNumInterfaces() const {
	return config->bNumInterfaces;
}

uint8_t	Configuration::bmAttributes() const {
	return config->bmAttributes;
}

uint8_t	Configuration::MaxPower() const {
	return config->MaxPower;
}

const std::vector<InterfacePtr>&	Configuration::interfaces() const {
	return interfacelist;
}

const InterfacePtr&	Configuration::operator[](size_t index) const {
	if (index >= interfacelist.size()) {
		throw std::range_error("outside interface range");
	}
	return interfacelist[index];
}

InterfacePtr&	Configuration::operator[](size_t index) {
	if (index >= interfacelist.size()) {
		throw std::range_error("outside interface range");
	}
	return interfacelist[index];
}

/**
 * \brief Create a list of interface objects for this configuration.
 *
 * This method scans the array of struct libusb_interface in the config
 * descriptor structure and constructs an Interface object for every
 * structure found.
 */
void	Configuration::getInterfaces() {
	for (int i = 0; i < config->bNumInterfaces; i++) {
		Interface	*ifd
			= new Interface(device(), *this, &config->interface[i],
				i);
		interfacelist.push_back(InterfacePtr(ifd));
	}
}

void	Configuration::configure() {
	dev.setConfiguration(bConfigurationValue());
}

static std::string	indent("C   ");

std::string	Configuration::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const Configuration& config) {
	out << indent << "bConfigurationValue:           ";
	out << (int)config.bConfigurationValue() << std::endl;
	out << indent << "bNumInterfaces:                ";
	out << (int)config.bNumInterfaces() << std::endl;
	out << indent << "bConfigurationValue:           ";
	out << (int)config.bConfigurationValue() << std::endl;
	out << indent << "bmAttributes:                  ";
	out << std::hex << (int)config.bmAttributes() << std::endl;
	out << indent << "MaxPower:                      ";
	out << std::dec << 2 * (int)config.MaxPower() << "mA" << std::endl;
	for (uint8_t ifno = 0; ifno < config.bNumInterfaces(); ifno++) {
		const InterfacePtr&	interface = config[ifno];
		out << *interface;
	}
	out << indent << "extra config data:             ";
	out << std::dec << (int)config.extra().size() << " bytes" << std::endl;
	return out;
}

} // namespace usb
} // namespace astro
