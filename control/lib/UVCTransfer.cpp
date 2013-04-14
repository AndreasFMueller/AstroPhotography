/*
 * UVCTransfer.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <sstream>
#include <iomanip>

namespace astro {
namespace usb {
namespace uvc {

std::string	stream_header2string(const stream_header_t& header) {
	std::ostringstream	out;
	out << "HLE:  " << (int)header.hle << std::endl;
	out << "BFH: ";
	if (0x80 & header.bfh) {
		out << " EOH";
	} else {
		out << " eoh";
	}
	if (0x40 & header.bfh) {
		out << " ERR";
	} else {
		out << " err";
	}
	if (0x10 & header.bfh) {
		out << " STI";
	} else {
		out << " sti";
	}
	if (0x20 & header.bfh) {
		out << " RES";
	} else {
		out << " res";
	}
	if (0x08 & header.bfh) {
		out << " SCR";
	} else {
		out << " scr";
	}
	if (0x04 & header.bfh) {
		out << " PTS";
	} else {
		out << " pts";
	}
	if (0x02 & header.bfh) {
		out << " EOF";
	} else {
		out << " eof";
	}
	if (0x01 & header.bfh) {
		out << " FID";
	} else {
		out << " fid";
	}
	out << std::endl;
	if (0x04 & header.bfh) {
		out << "PTS:  " << header.pts << std::endl;
	}
	if (0x08 & header.bfh) {
		out << "SCR: ";
		for (int i = 0; i < 6; i++) {
			out << " ";
			out << std::hex << std::setw(2) << std::setfill('0');
			out << (int)header.scr[i];
		}
		out << std::endl;
	}
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
