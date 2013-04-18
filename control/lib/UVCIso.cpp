/*
 * UVCIso.cpp -- implementations related to USB video class iso transfers
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>


namespace astro {
namespace usb {
namespace uvc {

UVCIsoPacket::UVCIsoPacket(const IsoPacket& isopacket) : IsoPacket(isopacket) {
	if (size() <= 12) {
		throw std::length_error("must have at least 12 bytes");
	}
}

uint8_t	UVCIsoPacket::hle() const {
	return (uint8_t)data()[0];
}

uint8_t	UVCIsoPacket::bfh() const {
	return (uint8_t)data()[1];
}

bool	UVCIsoPacket::eoh() const {
	return ((1 << 7) & (uint8_t)data()[1]) ? true : false;
}

bool	UVCIsoPacket::err() const {
	return ((1 << 6) & (uint8_t)data()[1]) ? true : false;
}

bool	UVCIsoPacket::sti() const {
	return ((1 << 5) & (uint8_t)data()[1]) ? true : false;
}

bool	UVCIsoPacket::res() const {
	return ((1 << 4) & (uint8_t)data()[1]) ? true : false;
}

bool	UVCIsoPacket::scr() const {
	return ((1 << 3) & (uint8_t)data()[1]) ? true : false;
}

bool	UVCIsoPacket::pts() const {
	return ((1 << 2) & (uint8_t)data()[1]) ? true : false;
}

bool	UVCIsoPacket::eof() const {
	return ((1 << 1) & (uint8_t)data()[1]) ? true : false;
}

bool	UVCIsoPacket::fid() const {
	return ((1 << 0) & (uint8_t)data()[1]) ? true : false;
}

uint32_t	UVCIsoPacket::ptsValue() const {
	return *(uint32_t *)&data()[2];
}

uint64_t	UVCIsoPacket::scrValue() const {
	uint64_t	result = 0;
	if (scr()) {
		memcpy(&result, &data()[7], 6);
	}
	return result;
}

std::string	UVCIsoPacket::payload() const {
	return substr((int)hle());
}

} // namespace uvc
} // namespace usb
} // namespace astro
