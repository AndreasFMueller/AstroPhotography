/*
 * MicroTouch.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MicroTouch.h>
#include <string.h>
#include <AstroDebug.h>
#include <iomanip>

using namespace astro::usb;

namespace astro {
namespace device {
namespace microtouch {

/**
 * \brief construct a microtouche exception instance
 */
MicroTouchError::MicroTouchError(const char *cause)
	: std::runtime_error(cause) {
}

typedef struct onebyte_s {
	uint8_t	result;
} __attribute__((packed)) onebyte_t;

/**
 * \brief Initialize the MicroTouch device
 *
 * The constructor also contains the initialization sequence of the
 * USB serial chip of the MicroTouch. Unfortunately, this chip is not open,
 * so I had to reverse engineer the initialization sequence using a USB
 * protocol analyzer. I have no idea what these commands mean, but they
 * work. The control requests in binary are displayed in the comments below.
 *
 * I assume that these control requests initialize the serial communication
 * with the AVR processor in the MicroTouch. As soon as they are sent, you
 * can just send and receive serial data on the buld endpoints the USB
 * serial chip also provides. 
 *
 * \param device USB device representing the MicroTouch
 */
MicroTouch::MicroTouch(DevicePtr _device) throw(USBError) : device(_device) {
	device->open();

	ConfigurationPtr	config = device->activeConfig();
	InterfacePtr	interfaceptr = (*config)[0];
	interfaceptr->claim();
	InterfaceDescriptorPtr	interfacedescriptorptr = (*interfaceptr)[0];
	inendpoint = (*interfacedescriptorptr)[0];
	outendpoint = (*interfacedescriptorptr)[1];
	std::cout << "IN:" <<std::endl;
	std::cout << *inendpoint;
	std::cout << "out:" <<std::endl;
	std::cout << *outendpoint;

	/* control request: 40 00 FF FF 00 00 00 00 */
	EmptyRequest	setup1(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		(uint16_t)0x0000, (uint8_t)0x00, (uint16_t)0xffff);
	device->controlRequest(&setup1);

	/* control request: 40 01 00 20 00 00 00 00 */
	EmptyRequest	setup2(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0x01, 0x2000);
	device->controlRequest(&setup2);

	/* control request: C0 FF 0B 37 00 00 01 00 */
	Request<onebyte_t>	setup3(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0xff, 0x370b);
	device->controlRequest(&setup3);

	/* control request: 40 12 0C 00 00 00 00 00 */
	EmptyRequest	setup4(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0x12, 0x000c);
	device->controlRequest(&setup4);

	/* control request: 40 01 C0 00 00 00 00 00 */
	EmptyRequest	setup5(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0x01, 0x00c0);
	device->controlRequest(&setup5);
}

template<size_t n>
struct mtdata {
	uint8_t	cmd;
	uint8_t	data[n];
	mtdata(uint8_t _cmd) { cmd = _cmd; }
	mtdata() { }
} __attribute__((packed));

/**
 * \brief get a word from the remote device
 *
 * \param code	command code to send to the device
 */
uint16_t	MicroTouch::getWord(uint8_t code) throw(MicroTouchError) {
	mtdata<2>	result = get<2>(code);
	uint16_t	word = result.data[1];
	word <<= 8;
	word += result.data[0];
	return word;
}

/**
 * \brief get a byte from the remote device
 *
 * \param code	command code to send to the device
 */
uint8_t	MicroTouch::getByte(uint8_t code) throw(MicroTouchError) {
	mtdata<1>	result = get<1>(code);
	return result.data[0];
}


/**
 * \brief Query the position from the MicroTouch.
 *
 * The position is returned as two a 16bit word when a GETPOSITION 
 * is sent to the MicroTouch device.
 *
 * \return The current stepper motor position.
 */
uint16_t	MicroTouch::position() throw(MicroTouchError) {
	return getWord(MICROTOUCH_GETPOSITION);
}

/**
 * \brief Find out whether Microtouch is moving
 * 
 * \return true if the motor is currently moving, false if not.
 */
bool	MicroTouch::isMoving() throw(MicroTouchError) {
	return getByte(MICROTOUCH_ISMOVING) ? true : false;
}

/**
 * \brief  Find out whether temperature compensation is turned on.
 *
 * \return true if temperature consation is turned on, false if not.
 */
bool	MicroTouch::isTemperatureCompensating() throw(MicroTouchError) {
	return getByte(MICROTOUCH_ISTEMPCOMPENSATING) ? true : false;
}

/**
 * \brief Drive the stepper motor to a given position.
 *
 * To set the position on the microtouch, one has to send the SETPOSITION
 * command byte followed by 4 bytes indicating the position. The first
 * byte is the result of the integer division position / 1000. The next 
 * three bytes are the last three decimal digits of position, in binary.
 *
 * \param position	the position to drive to.
 */
void	MicroTouch::setPosition(uint16_t position) throw(MicroTouchError) {
	// send a command to the MicroTouch
	debug(LOG_DEBUG, DEBUG_LOG, 0, "send position request");
	mtdata<4>	request_data(MICROTOUCH_SETPOSITION);
	int	r, p;
	p = position;
	// last three decimal digits are the first three command bytes
	r = p % 10;
	p = (p - r) / 10;
	request_data.data[0] = r;
	r = p % 10;
	p = (p - r) / 10;
	request_data.data[1] = r;
	r = p % 10;
	p = (p - r) / 10;
	request_data.data[2] = r;
	// the quotient by 1000 gives the last byte of the command
	std::cout << "p = " << p << std::endl;
	request_data.data[3] = p;
	BulkTransfer	request(outendpoint, &request_data);
	device->submit(&request);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transmit complete");
}

/**
 * \brief Get the current temperature
 *
 * \return give the temperature of the hand controller in degrees Celsius.
 */
float	MicroTouch::getTemperature() throw(MicroTouchError) {
	// send a command to the MicroTouch
	mtdata<5>	result = get<5>(MICROTOUCH_GETTEMPERATURE);

	// a 0 in byte 2 indicates a problem
	if (result.data[2] == 0) {
		throw MicroTouchError("no temperature info");
	}

	// convert the response to number
	int16_t	temp = result.data[0];
	temp = (temp << 8) | result.data[1];
	int16_t	offset = result.data[4];
	offset = (offset << 8) | result.data[3];
	double temperature = (temp + offset) / 16.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got temperature %.1f", temperature);
	return temperature;
}

/**
 * \brief Start moving up
 */
void	MicroTouch::stepUp() throw(MicroTouchError) {
	mtdata<0>	stepup_request(MICROTOUCH_STARTUP);
	BulkTransfer	request(outendpoint, &stepup_request);
	device->submit(&request);
}



} // namespace microtouch
} // namepsace device
} // namepsace astro
