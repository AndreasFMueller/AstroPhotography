/*
 * MicroTouch.h -- driver for Microtouch focuser motor
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _MicroTouch_h
#define _MicroTouch_h

#include <AstroUSB.h>
#include <stdexcept>

namespace astro {
namespace device {
namespace microtouch {

/*
 * Here are the command bytes I was able to reveres engineer
 */
#define MICROTOUCH_ISMOVING		0x82
#define	MICROTOUCH_GETTEMPERATURE	0x84
#define MICROTOUCH_ISTEMPCOMPENSATING	0x89
#define MICROTOUCH_SETPOSITION		0x8c
#define MICROTOUCH_GETPOSITION		0x8d
#define MICROTOUCH_STARTUP		0x8e

/**
 * \brief Exception class for MicroTouch
 */
class MicroTouchError : public std::runtime_error {
public:
	MicroTouchError(const char *cause);
};

/**
 * \brief 
 */
class MicroTouch {
	astro::usb::DevicePtr	device;
	astro::usb::EndpointDescriptorPtr	outendpoint;
	astro::usb::EndpointDescriptorPtr	inendpoint;

	template<size_t n>
	struct mtdata {
		uint8_t	cmd;
		uint8_t	data[n];
		mtdata(uint8_t _cmd) { cmd = _cmd; }
		mtdata() { }
	} __attribute__((packed));

	/**
 	 * \brief Retrieve a block of data from the Microtouch device
	 *
	 * This private template function is sends a command byte to the
	 * microtouch device and returns a number of bytes received from
	 * the device as a response. The template argument n is the size
	 * of the data block to receive.
	 *
	 * This method is also used to send commands with arguments. In this
	 * case, it is used with n=0 so that only the command byte is sent,
	 * and a bulk transfer is used to send the arguments.
	 *
	 * \param code	command byte to send to the device
 	 */
	template<size_t n>
	mtdata<n>	get(uint8_t code) throw(MicroTouchError) {
		mtdata<0>	request_data(code);
		astro::usb::BulkTransfer	request(outendpoint, &request_data);
		device->submit(&request);
		mtdata<n>	response_data;
		astro::usb::BulkTransfer	response(inendpoint, &response_data);
		device->submit(&response);
		if (request_data.cmd != response_data.cmd) {
			throw MicroTouchError("response command code mismatch");
		}
		return response_data;
	}

public:
	MicroTouch(astro::usb::DevicePtr device) throw(astro::usb::USBError);
	uint16_t	getWord(uint8_t code) throw(MicroTouchError);
	uint16_t	position() throw(MicroTouchError);
	void		setPosition(uint16_t position) throw(MicroTouchError);

	uint8_t		getByte(uint8_t code) throw(MicroTouchError);
	bool	isMoving() throw(MicroTouchError);
	bool	isTemperatureCompensating() throw(MicroTouchError);

	float	getTemperature() throw(MicroTouchError);

	void	stepUp() throw(MicroTouchError);
};

} // namespace microtouch
} // namespace device
} // namespace astro

#endif /* _MicroTouch_h */
