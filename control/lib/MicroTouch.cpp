/*
 * MicroTouch.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MicroTouch.h>
#include <string.h>

using namespace astro::usb;

namespace astro {
namespace microtouch {

typedef struct onebyte_s {
	uint8_t	result;
} __attribute__((packed)) onebyte_t;

MicroTouch::MicroTouch(Device& _device) : device(_device) {
	device.open();

	/* 40 00 FF FF 00 00 00 00 */
	EmptyRequest	setup1(RequestBase::vendor_specific_type,
		(uint16_t)0xffff, (uint8_t)0x00, (uint16_t)0x0000);
	device.controlRequest(&setup1);

	/* 40 01 00 20 00 00 00 00 */
	EmptyRequest	setup2(RequestBase::vendor_specific_type,
		0x0000, 0x01, 0x0020);
	device.controlRequest(&setup2);

	/* C0 FF 0B 37 00 00 01 00 */
	Request<onebyte_t>	setup3(RequestBase::vendor_specific_type,
		0x0000, 0xff, 0x370b);
	device.controlRequest(&setup3);

	/* 40 12 0C 00 00 00 00 00 */
	EmptyRequest	setup4(RequestBase::vendor_specific_type,
		0x0000, 0x12, 0x000c);
	device.controlRequest(&setup4);

	/* 40 01 C0 00 00 00 00 00 */
	EmptyRequest	setup5(RequestBase::vendor_specific_type,
		0x0000, 0x01, 0x00c0);
	device.controlRequest(&setup5);
}


uint16_t	MicroTouch::position() {
	// send a position request to the MicroTouch
	unsigned char	position_request = 0x8d;
	TransferPtr	preq(new BulkTransfer(0x01, 1, &position_request));
	device.submit(preq);

	// read the position back
	unsigned char	position_response[3];
	TransferPtr	presp(new BulkTransfer(0x81, 1, position_response));
	device.submit(presp);

	// when the transport is complete, we should have the code
	// 0x8d in the first byte
	if (position_response[0] != position_request) {
		throw std::runtime_error("bad command response");
	}

	return *(uint16_t *)&position_response[3];
}

bool	MicroTouch::isMoving() {
	return false;
}

void	MicroTouch::stepUp() {
std::cout << "step up" << std::endl;
	unsigned char	stepup_request = 0x8e;
	TransferPtr	transfer(new BulkTransfer(0x01, 1, &stepup_request));
	device.submit(transfer);
}



} // namespace microtouch
} // namepsace astro