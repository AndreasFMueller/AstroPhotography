/*
 * MicroTouch.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MicroTouch.h>
#include <string.h>
#include <debug.h>
#include <iomanip>

using namespace astro::usb;

namespace astro {
namespace microtouch {

typedef struct onebyte_s {
	uint8_t	result;
} __attribute__((packed)) onebyte_t;

/**
 * \brief Initialize the MicroTouch device
 *
 * \param device USB device representing the MicroTouch
 */
MicroTouch::MicroTouch(Device& _device) : device(_device) {
	device.open();

	ConfigurationPtr	config = device.activeConfig();
	InterfacePtr	interfaceptr = (*config)[0];
	interfaceptr->claim();
	InterfaceDescriptorPtr	interfacedescriptorptr = (*interfaceptr)[0];
	inendpoint = (*interfacedescriptorptr)[0];
	outendpoint = (*interfacedescriptorptr)[1];
	std::cout << "IN:" <<std::endl;
	std::cout << *inendpoint;
	std::cout << "out:" <<std::endl;
	std::cout << *outendpoint;

	/* 40 00 FF FF 00 00 00 00 */
	EmptyRequest	setup1(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		(uint16_t)0x0000, (uint8_t)0x00, (uint16_t)0xffff);
	device.controlRequest(&setup1);

	/* 40 01 00 20 00 00 00 00 */
	EmptyRequest	setup2(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0x01, 0x2000);
	device.controlRequest(&setup2);

	/* C0 FF 0B 37 00 00 01 00 */
	Request<onebyte_t>	setup3(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0xff, 0x370b);
	device.controlRequest(&setup3);

	/* 40 12 0C 00 00 00 00 00 */
	EmptyRequest	setup4(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0x12, 0x000c);
	device.controlRequest(&setup4);

	/* 40 01 C0 00 00 00 00 00 */
	EmptyRequest	setup5(RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		0x0000, 0x01, 0x00c0);
	device.controlRequest(&setup5);
}

/**
 * \brief get a word from the remote device
 */
uint16_t	MicroTouch::getWord(uint8_t code) {
	// send a command to the MicroTouch
	debug(LOG_DEBUG, DEBUG_LOG, 0, "send %02x request", code);
	uint8_t	request_data = code;
	BulkTransfer	request(outendpoint, 1, &request_data);
	device.submit(&request);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transmit complete");

	// read the result back
	debug(LOG_DEBUG, DEBUG_LOG, 0, "receive  request");
	uint8_t	response_data[3];
	BulkTransfer	response(inendpoint, 3, response_data);
	device.submit(&response);

	// when the transport is complete, we should have the code
	// 0x8d in the first byte
	if (response_data[0] != request_data) {
		throw std::runtime_error("bad command response");
	}

	return *(uint16_t *)&response_data[1];
}

/**
 * \brief get a byte from the remote device
 */
uint8_t	MicroTouch::getByte(uint8_t code) {
	// send a command to the MicroTouch
	debug(LOG_DEBUG, DEBUG_LOG, 0, "send %02x request", code);
	uint8_t	request_data = code;
	BulkTransfer	request(outendpoint, 1, &request_data);
	device.submit(&request);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transmit complete");

	// read the result back
	debug(LOG_DEBUG, DEBUG_LOG, 0, "receive  request");
	uint8_t	response_data[2];
	BulkTransfer	response(inendpoint, 2, response_data);
	device.submit(&response);

	// when the transport is complete, we should have the code
	// 0x8d in the first byte
	if (response_data[0] != request_data) {
		throw std::runtime_error("bad command response");
	}

	return response_data[1];
}


/**
 * \brief Query the position from the MicroTouch.
 */
uint16_t	MicroTouch::position() {
	return getWord(0x8d);
}

/**
 * \brief Find out whether Microtouch is moving
 */
bool	MicroTouch::isMoving() {
	return getByte(0x82) ? true : false;
}

bool	MicroTouch::isTemperatureCompensating() {
	return getByte(0x89) ? true : false;
}

void	MicroTouch::setPosition(uint16_t position) {
	// send a command to the MicroTouch
	debug(LOG_DEBUG, DEBUG_LOG, 0, "send position request");
	uint8_t	request_data[5] = { 0x8c, 0x00, 0x00, 0x00, 0x00 };
	int	r, p;
	p = position;
	r = p % 10;
	p = (p - r) / 10;
	request_data[1] = r;
	r = p % 10;
	p = (p - r) / 10;
	request_data[2] = r;
	r = p % 10;
	p = (p - r) / 10;
	request_data[3] = r;
	std::cout << "p = " << p << std::endl;
	request_data[4] = p;
	BulkTransfer	request(outendpoint, 5, request_data);
	device.submit(&request);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transmit complete");
}

/**
 * \brief Get the current temperature
 */
float	MicroTouch::getTemperature() {
	// send a command to the MicroTouch
	uint8_t	code = 0x84;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "send %02x request", code);
	uint8_t	request_data = code;
	BulkTransfer	request(outendpoint, 1, &request_data);
	device.submit(&request);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transmit complete");

	// read the result back
	debug(LOG_DEBUG, DEBUG_LOG, 0, "receive  request");
	uint8_t	response_data[6];
	BulkTransfer	response(inendpoint, 6, response_data);
	device.submit(&response);

	// when the transport is complete, we should have the code
	// 0x8d in the first byte
	if (response_data[0] != request_data) {
		throw std::runtime_error("bad command response");
	}
	for (int i = 0; i < 6; i++) {
		std::cout << " " << std::hex << std::setw(2);
		std::cout << std::setfill('0') << (int)response_data[i];
	}
	std::cout << std::endl;

	uint16_t	temp = response_data[1];
	temp = (temp << 8) | response_data[2];
	return 0.1 * (temp + 368);
}

/**
 * \brief Start moving up
 */
void	MicroTouch::stepUp() {
std::cout << "step up" << std::endl;
	unsigned char	stepup_request = 0x8e;
	BulkTransfer	request(outendpoint, 1, &stepup_request);
	device.submit(&request);
}



} // namespace microtouch
} // namepsace astro
