/*
 * USBInterface.cpp -- Interface and InterfaceDescriptor implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <sstream>
#include <ios>
#include <iomanip>
#include <string.h>
#include <AstroDebug.h>

namespace astro {
namespace usb {

/////////////////////////////////////////////////////////////////////
// Interface Descriptor implementation
/////////////////////////////////////////////////////////////////////

void	InterfaceDescriptor::copy(const struct libusb_interface_descriptor *_ifdp) {
	ifdp = new struct libusb_interface_descriptor;
	memcpy(ifdp, _ifdp, sizeof(struct libusb_interface_descriptor));
	ifdp->extra = NULL;
	ifdp->extra_length = 0;
}

InterfaceDescriptor::InterfaceDescriptor(Device& device,
	Interface& _interface, const struct libusb_interface_descriptor *_ifdp)
		: Descriptor(device, _ifdp->extra, _ifdp->extra_length),
		  interface(_interface) {
	copy(_ifdp);
	getEndpoints();
	interfacename = dev.getStringDescriptor(ifdp->iInterface);
}

uint8_t	InterfaceDescriptor::bInterfaceNumber() const {
	return ifdp->bInterfaceNumber;
}

uint8_t	InterfaceDescriptor::bAlternateSetting() const {
	return ifdp->bAlternateSetting;
}

uint8_t	InterfaceDescriptor::bInterfaceClass() const {
	return ifdp->bInterfaceClass;
}

uint8_t	InterfaceDescriptor::bInterfaceSubClass() const {
	return ifdp->bInterfaceSubClass;
}

uint8_t	InterfaceDescriptor::bInterfaceProtocol() const {
	return ifdp->bInterfaceProtocol;
}

const std::string& 	InterfaceDescriptor::iInterface() const {
	return interfacename;
}

void	InterfaceDescriptor::getEndpoints() {
	for (int i = 0; i < ifdp->bNumEndpoints; i++) {
		EndpointDescriptor	*epp
			= new EndpointDescriptor(device(), *this,
				&ifdp->endpoint[i]);
		endpointlist.push_back(EndpointDescriptorPtr(epp));
	}
}

int	InterfaceDescriptor::numEndpoints() const {
	return endpointlist.size();
}

EndpointDescriptorPtr	InterfaceDescriptor::operator[](size_t index) const {
	if ((index < 0) || (index >= endpointlist.size())) {
		throw std::range_error("outside endpoint range");
	}
	return endpointlist[index];
}

void	InterfaceDescriptor::altSetting() throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select alt setting %d on interface %d",
		bAlternateSetting(), bInterfaceNumber());
	dev.setInterfaceAltSetting(bInterfaceNumber(), bAlternateSetting());
}

static std::string	indent("        A   ");

std::string	InterfaceDescriptor::toString() const {
	std::ostringstream	out;
	out << indent << "bInterfaceNumber:      ";
	out << (int)bInterfaceNumber() << std::endl;
	out << indent << "bAlternateSetting:     ";
	out << (int)bAlternateSetting() << std::endl;
	out << indent << "bInterfaceClass:       ";
	out << (int)bInterfaceClass() << std::endl;
	out << indent << "bInterfaceSubClass:    ";
	out << (int)bInterfaceSubClass() << std::endl;
	out << indent << "bInterfaceProtocol:    ";
	out << (int)bInterfaceProtocol() << std::endl;
	out << indent << "iInterface:            ";
	out << iInterface() << std::endl;
	out << indent << "Endpoints:             ";
	if (0 == numEndpoints()) {
		out << "none";
	} else {
		out << (int)numEndpoints();
	}
	out << std::endl;
	std::vector<EndpointDescriptor>::const_iterator	i;
	for (int endpointno = 0; endpointno < numEndpoints(); endpointno++) {
		out << indent << "Endpoint " << endpointno << ":" << std::endl;
		out << *(*this)[endpointno];
	}
	out << indent << "extra interface data:  ";
	out << std::dec << (int)extra().size() << " bytes" << std::endl;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const InterfaceDescriptor& ifd) {
	return out << ifd.toString();
}

Interface&	InterfaceDescriptor::getInterface() {
	return interface;
}

/////////////////////////////////////////////////////////////////////
// Interface implementation
/////////////////////////////////////////////////////////////////////

Interface::Interface(Device& device, Configuration& _configuration,
	const libusb_interface *li, int _interfaceindex)
	: dev(device), interfaceindex(_interfaceindex),
	  configuration(_configuration) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating interface index=%d",
		interfaceindex);
	for (int i = 0; i < li->num_altsetting; i++) {
		InterfaceDescriptor	*id
			= new InterfaceDescriptor(device, *this,
				&li->altsetting[i]);
		altsettingvector.push_back(InterfaceDescriptorPtr(id));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interface with index %d has number %d",
			interfaceindex, interfaceNumber());
	reattach = false;
}

Interface::~Interface() {
	try {
		if (reattach) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"reattach kernel driver");
			attachKernelDriver();
		}
	} catch (USBError& error) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during kernel driver "
			"reattach: %s", error.what());
	}
}

uint8_t	Interface::interfaceNumber() const {
	return altsettingvector[0]->bInterfaceNumber();
}

size_t	Interface::numAltsettings() const {
	return altsettingvector.size();
}

const InterfaceDescriptorPtr&	Interface::operator[](size_t index) const {
	if (index >= numAltsettings()) {
		throw std::range_error("out of alt setting range");
	}
	return altsettingvector[index];
}

InterfaceDescriptorPtr&	Interface::operator[](size_t index) {
	if ((index < 0) || (index >= numAltsettings())) {
		throw std::range_error("out of alt setting range");
	}
	return altsettingvector[index];
}

void	Interface::claim() throw(USBError) {
	dev.claimInterface(interfaceNumber());
}

void	Interface::release() throw(USBError) {
	dev.releaseInterface(interfaceNumber());
}

static std::string	ifindent("    I   ");

std::string	Interface::toString() const {
	std::ostringstream	out;
	out << ifindent << "Interface " << (int)(*this)[0]->bInterfaceNumber()
		<< " with " << numAltsettings();
	out << " alternate settings:" << std::endl;;
	for (size_t j = 0; j < numAltsettings(); j++) {
		out << *(*this)[j];
	}
	out << ifindent << "end interface" << std::endl;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const Interface& interface) {
	return out << interface.toString();
}

bool	Interface::kernelDriverActive() const {
	return dev.kernelDriverActive(interfaceNumber());
}

/**
 * \brief Detach a kernel driver.
 *
 * If a kernel driver is active, then this method detaches it. When the
 * Interface is deallocated, then the kernel driver is reattached.
 */
void	Interface::detachKernelDriver() throw(USBError) {
	if (kernelDriverActive()) {
		reattach = true;
	} else {
		return;
	}
	dev.detachKernelDriver(interfaceNumber());
}

void	Interface::attachKernelDriver() const throw(USBError) {
	dev.attachKernelDriver(interfaceNumber());
}

} // namespace usb
} // namespace astro
