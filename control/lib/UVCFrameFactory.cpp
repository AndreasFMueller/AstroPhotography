/*
 * UVCFrameFactory.cpp -- build a vector of frames from a list of payload
 *                        packets
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <debug.h>

namespace astro {
namespace usb {
namespace uvc {

FrameFactory::FrameFactory(int _width, int _height, int _bytesperpixel)
	: width(_width), height(_height), bytesperpixel(_bytesperpixel) {
}

/**
 * \brief Extract a vector of frames from a packet list
 *
 * \param packets     a list of packets retrieved through a payload transfer
 */
std::vector<FramePtr>	FrameFactory::operator()(const std::list<std::string>& packets)
	const {
	// prepare a vector of frames that we can later return
	std::vector<FramePtr>	frames;
	Frame	*currentframe = new Frame(width, height);
	int	packetcounter = 0;
	int	processed = 0;
	int	framecounter = 0;

	// compute the size of frames that we expect
	int	minsize = width * height * bytesperpixel;

	// go through the packet list and put together all the data
	bool	fid = false;
	std::list<std::string>::const_iterator	i;
	for (i = packets.begin(); i != packets.end(); i++) {
		try {
			UVCPayloadPacket	uvcpayload(*i);
			if (i->length() > 12) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "%d: %u, %u, %u, %d",
					(int)uvcpayload.hle(), 
					uvcpayload.ptsValue(), 
					uvcpayload.fid(), 
					uvcpayload.pts(),
					i->length() - 12);
					
			}
			if (uvcpayload.fid() == fid) {
				if (NULL == currentframe) {
					currentframe = new Frame(width, height);
				}
				currentframe->append(uvcpayload.payload());
			} else {
				if ((currentframe) && (currentframe->size() >= minsize)) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"adding frame of size %d",
						currentframe->size());
					frames.push_back(FramePtr(currentframe));
					framecounter++;
				}
				currentframe = new Frame(width, height);
				currentframe->append(uvcpayload.payload());
				fid = uvcpayload.fid();
			}
			processed++;
		} catch (std::exception& x) {
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "packet %d ignored: %s",
			//	packetcounter, x.what());
		}
		packetcounter++;
	}
	if (currentframe) {
		delete currentframe;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processed packets: %d, frames: %d",
		processed, framecounter);

	// show how large the frames are:
	std::vector<FramePtr>::const_iterator	j;
	framecounter = 0;
	for (j = frames.begin(); j != frames.end(); j++, framecounter++) {
#if 1
		debug(LOG_DEBUG, DEBUG_LOG, 0, "frame %d: %d bytes",
			framecounter, (*j)->size());
#endif
	}
	if (framecounter == 0) {
		throw std::length_error("no frames received");
	}

	// return all the frames
	return	frames;
}

} // namespace uvc
} // namespace usb
} // namespace astro
