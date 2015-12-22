/*
 * QhyUtils.cpp -- utility functions for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief Parse the name into libusb bus number and device address
 *
 * Parse the QHY name into the individual fields of the name. These are:
 * the USB bus number, the USB device address, human readable product name,
 * USB vendor id in hex, USB product id in hex, and if present a serial
 * number.
 */
void	QhyName::parse(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing name '%s'", name.c_str());
	// USB bus number
	std::string	busnumberstring = name.substr(0, 3);
	_busnumber = stoi(busnumberstring);
	// USB device address
	std::string	deviceaddressstring = name.substr(4,3);
	_deviceaddress = stoi(deviceaddressstring);
	// human readable product name
	std::string	w = name.substr(8);
	size_t	o = w.find('-', 4);
	iproduct = w.substr(0, o);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iproduct = %s", iproduct.c_str());
	// get the rest of the name
	w = w.substr(o + 1);
	// idvendor
	idvendor = stoi(w.substr(0, 4), 0, 16);
	// idproduct
	idproduct = stoi(w.substr(5, 4), 0, 16);
	// check for the serial number
	o = w.find('-', 10);
	if (std::string::npos != o) {
		serial = w.substr(o + 1);
	} else {
		serial = "";
	}
	
	// find the next dash to extract idvendor and idproduct
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has bus=%d, addr=%d, iprod=%s, "
		"idvendor=%04x, idproduct=%04x, serial=%s", name.c_str(),
		_busnumber, _deviceaddress, iproduct.c_str(),
		idvendor, idproduct, serial.c_str());
}

/**
 * \brief Construct a QHY name based on the USB device ptr
 *
 * \param deviceptr	USB device ptr to construct the 
 */
QhyName::QhyName(astro::usb::DevicePtr deviceptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "QhyName constructor on USB: %s",
		deviceptr->getBusAndAddress().c_str());
	usb::DeviceDescriptorPtr	descriptor = deviceptr->descriptor();
	if (QHY_VENDOR_ID != descriptor->idVendor()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "device is not a QHY device");
		throw std::runtime_error("not an QHY device");
	}
	_busnumber = deviceptr->getBusNumber();
	_deviceaddress = deviceptr->getDeviceAddress();
	iproduct = trim(descriptor->iProduct());
	if (descriptor->iSerialNumber().size() > 0) {
		serial = trim(descriptor->iSerialNumber());
	}
	idvendor = descriptor->idVendor(),
	idproduct = descriptor->idProduct();
}

/**
 * \brief Construct a QHY name basd on the device name
 */
QhyName::QhyName(const DeviceName& devicename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "QhyName constructor on DeviceName: %s",
		devicename.toString().c_str());
	if (devicename[0] != "qhy") {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s ist not a QHY device",
			devicename.toString().c_str());
		throw std::runtime_error("not a QHY device");
	}
	parse(devicename[1]);
}

std::string	QhyName::unparse() const {
	std::string	name = stringprintf(
		"%03d-%03d-%s-%04x-%04x",
		_busnumber, _deviceaddress, iproduct.c_str(),
		idvendor, idproduct);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name = %s", name.c_str());
	if (serial.size() > 0) {
		name.append("-");
		name.append(serial);
	}
	return name;
}

DeviceName	QhyName::name(DeviceName::device_type type) const {
	std::vector<std::string>	components;
	components.push_back("qhy");
	components.push_back(unparse());
	return DeviceName(type, components);
}

std::string	QhyName::stringname(DeviceName::device_type type) const {
	return name(type).toString();
}

DeviceName	QhyName::name(DeviceName::device_type type,
			const std::string& path) const {
	std::vector<std::string>	components;
	std::string	work = path;
	size_t	offset;
	do {
		std::string	component;
		offset = work.find('/');
		if (std::string::npos != offset) {
			component = work.substr(0, offset);
			work = work.substr(offset + 1);
		} else {
			component = work;
			work = "";
		}
		if (component.size() > 0) {
			components.push_back(component);
		}
	} while (work.size() > 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d components in %s",
		components.size(), path.c_str());
	return name(type, components);
}

std::string	QhyName::stringname(DeviceName::device_type type,
			const std::string& path) const {
	return name(type, path).toString();
}

DeviceName	QhyName::name(DeviceName::device_type type,
			const std::vector<std::string>& path) const {
	std::vector<std::string>	components;
	components.push_back("qhy");
	components.push_back(unparse());
	std::vector<std::string>::const_iterator	i;
	for (i = path.begin(); i != path.end(); i++) {
		components.push_back(*i);
	}
	return DeviceName(type, components);
}

std::string	QhyName::stringname(DeviceName::device_type type,
			const std::vector<std::string>& path) const {
	return name(type, path).toString();
}

DeviceName	QhyName::cameraname() const {
	return name(DeviceName::Camera);
}

DeviceName	QhyName::ccdname() const {
	return name(DeviceName::Ccd, "Imaging");
}

DeviceName	QhyName::coolername() const {
	return name(DeviceName::Cooler, "Imaging/cooler");
}

DeviceName	QhyName::guiderportname() const {
	return name(DeviceName::Guiderport, "guiderport");
}

bool	QhyName::matches(const DeviceName& other, DeviceName::device_type type) {
	if (other[0] != "qhy") {
		return false;
	}
	if (unparse() != other[1]) {
		return false;
	}
	if (type != other.type()) {
		return false;
	}
	return true;
}

bool	QhyName::isCamera(const DeviceName& other) {
	if (!matches(other, DeviceName::Camera)) {
		return false;
	}
	if (other.size() != 2) {
		return false;
	}
	return true;
}

bool	QhyName::isCcd(const DeviceName& other) {
	if (!matches(other, DeviceName::Ccd)) {
		return false;
	}
	if (3 != other.size()) {
		return false;
	}
	return (other[2] == "Imaging");
}

bool	QhyName::isCooler(const DeviceName& other) {
	if (!matches(other, DeviceName::Cooler)) {
		return false;
	}
	if (4 != other.size()) {
		return false;
	}
	return ((other[2] == "Imaging") && (other[3] == "cooler"));
}

bool	QhyName::isGuiderport(const DeviceName& other) {
	if (!matches(other, DeviceName::Guiderport)) {
		return false;
	}
	if (3 != other.size()) {
		return false;
	}
	return (other[2] == "guiderport");
}

} // namespace qhy
} // namespace camera
} // namespace astro
