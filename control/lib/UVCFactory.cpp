/*
 * UVCFactory.cpp -- Factories for the UVC control or streaming descriptors
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschuel Rappersil
 */
#include <AstroUVC.h>
#include <sstream>
#include <debug.h>

using namespace astro::usb;

namespace astro {
namespace usb {
namespace uvc {

//////////////////////////////////////////////////////////////////////
// UVCDescriptorFactory
//////////////////////////////////////////////////////////////////////
UVCDescriptorFactory::UVCDescriptorFactory(Device& _device)
	: DescriptorFactory(_device) {
}

uint8_t	UVCDescriptorFactory::bdescriptorsubtype(const void *data) const {
	return ((uint8_t *)data)[2];
}

USBDescriptorPtr	UVCDescriptorFactory::descriptor(const void *data,
	int length)
	throw(std::length_error, UnknownDescriptorError) {
	// if there is not enough data even to find out what type of
	// descriptor comes next, then this is an error
	if (length < 2) {
		throw std::length_error("not engouth data for descriptor");
	}
	// check that there is enough data to process
	if (blength(data) > length) {
		throw std::length_error("not enough data for descriptor");
	}
#if 0
	// currently not used
	uint8_t	type = bdescriptortype(data);
	uint8_t	subtype = bdescriptorsubtype(data);
#endif

	return DescriptorFactory::descriptor(data, length);
}

//////////////////////////////////////////////////////////////////////
// VideoControlDescriptorFactory
//////////////////////////////////////////////////////////////////////
VideoControlDescriptorFactory::VideoControlDescriptorFactory(
	Device& _device)
	: UVCDescriptorFactory(_device) {
}

uint16_t	VideoControlDescriptorFactory::wterminaltype(const void *data)
			const {
	return *(uint16_t *)&(((uint8_t *)data)[4]);
}

/**
 * \brief Parse a header descriptor and all the attached video control unit
 *        descriptors.
 *
 * A InterfaceHeaderDescriptor never comes alone, it is always accompanied
 * by a sequence fo video control unit descriptors. For camera control, we
 * only need two headers: the camera terminal descriptor and the processing
 * unit descriptor. However, we still parse all the headers.
 */
USBDescriptorPtr	VideoControlDescriptorFactory::header(
				const void *data, int length) {
	// create the header
	InterfaceHeaderDescriptor	*ifhd = new InterfaceHeaderDescriptor(
		device, data, length);

	// add the units
	int	offset = ifhd->bLength();
	while (offset < length) {
		USBDescriptorPtr	unit
			= descriptor(offset + (uint8_t *)data, length - offset);
		ifhd->units.push_back(unit);
		offset += unit->bLength();
	}

	// make sure we know about the camera and the processing unit
	// controls
	ifhd->getIds();

	// return the InterfaceHeader
	return USBDescriptorPtr(ifhd);
}

/**
 * \brief Main video control descriptor parser function
 *
 * On ertain descriptors, most notably the video control header descriptor,
 * this method calls other methods that will parse other headers attached
 * to the first.
 */
USBDescriptorPtr	VideoControlDescriptorFactory::descriptor(
	const void *data, int length)
	throw(std::length_error, UnknownDescriptorError) {

	// if there is not enough data even to find out what type of
	// descriptor comes next, then this is an error
	if (length < 2) {
		throw std::length_error("not engouth data for descriptor");
	}

	// check that there is enough data to process
	if (blength(data) > length) {
		throw std::length_error("not enough data for descriptor");
	}

	// make sure we are in an interface descriptor
	uint8_t	type = bdescriptortype(data);
	if (type != CS_INTERFACE) {
		throw std::runtime_error("not in an interface descriptor");
	}

	// get the subtype
	uint8_t	subtype = bdescriptorsubtype(data);

	// we now have all the information to start building the descriptors
	USBDescriptorPtr	result;
	switch (subtype) {
	case VC_HEADER:
		result = header(data, length);
		break;
	case VC_OUTPUT_TERMINAL:
		result = USBDescriptorPtr(new OutputTerminalDescriptor(
			device, data, length));
		break;
	case VC_INPUT_TERMINAL:
		if (wterminaltype(data) == ITT_CAMERA) {
			result = USBDescriptorPtr(
				new CameraTerminalDescriptor(
					device, data, length));
		} else {
			result = USBDescriptorPtr(
				new InputTerminalDescriptor(
					device, data, length));
		}
		break;
	case VC_SELECTOR_UNIT:
		result = USBDescriptorPtr(new SelectorUnitDescriptor(
				device, data, length));
		break;
	case VC_PROCESSING_UNIT:
		result = USBDescriptorPtr(new ProcessingUnitDescriptor(
				device, data, length));
		break;
	case VC_EXTENSION_UNIT:
		result = USBDescriptorPtr(new ExtensionUnitDescriptor(
				device, data, length));
		break;
	default:
		throw UnknownDescriptorError(type, subtype);
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// VideoStreamingDescriptorFactory
//////////////////////////////////////////////////////////////////////
VideoStreamingDescriptorFactory::VideoStreamingDescriptorFactory(
	Device& _device)
	: UVCDescriptorFactory(_device) {
}

/**
 * \brief Parse the header descriptor with all the associated format descriptor
 *
 * This method scans the data block for format descriptors following the
 * header descriptor at the beginning of the block.
 * \param data		data block containing the descriptors
 * \param length	length of the data block
 * \param hd		Header descriptor constructed from the data
 */
USBDescriptorPtr	VideoStreamingDescriptorFactory::header(
				const void *data, int length,
				HeaderDescriptor *hd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "completing a header descriptor");
	// add the formats
	int	offset = hd->bLength();
	int	nformats = hd->bNumFormats(); // for TIS, we should not rely on
					      // this, and we change nformats
					      // below if we find more formats
	int	formatindex = 0;
	for (; formatindex < nformats; formatindex++) {
		// use the descriptor method of the factory to get the
		// next desriptor. This also parses the format headers
		debug(LOG_DEBUG, DEBUG_LOG, 0, "FO parse format %d", formatindex);
		USBDescriptorPtr	newformat
			= descriptor(offset + (uint8_t *)data, length - offset);

		// check that it really is a format descriptor
		FormatDescriptor	*fd
			= dynamic_cast<FormatDescriptor *>(&*newformat);
		if (NULL == fd) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "not a format");
			throw std::runtime_error("expected a FormatDescriptor");
		}
		hd->formats.push_back(newformat);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "FO new format found");

		// find out how long that format is and go to the next
		// descriptor
		offset += fd->wTotalLength();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "total length: %d, new offset %d",
			fd->wTotalLength(), offset);

		// it is possible that there are still image frame descriptors
		// or color matching descriptors following the frames, so
		// we should skip them and add to the total length of the
		// format descriptor
		if (length > offset) {
			do {
				// get the next descriptor
				newformat = descriptor(offset + (uint8_t *)data,
					length - offset);

				// if it is a format descriptor, then we have
				// seen all descriptors, and should leave
				// the loop
				fd = dynamic_cast<FormatDescriptor *>(&*newformat);

				// if it is not a format descriptor, then we
				// should add it to the total length, and go
				// to the next descriptor
				if (fd == NULL) {
					offset += newformat->bLength();
				}
			} while ((fd == NULL) && (length > offset));
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"unknown descriptors skipped");
		}

		// at this point we have either are at the end of the
		// the extra descriptors, or there are more format
		// descriptors. But we use this logic only if this is a TIS
		// camera
		if (device.getBroken() == BROKEN_THE_IMAGING_SOURCE) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "fixing nformats: %d",
				nformats);
			// there must be more room, formatindex is the last
			// index
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"length = %d > offset = %d", length, offset);
			if (length > offset) {
				if (formatindex == (nformats - 1)) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"FO expect another format");
					nformats++;
				}
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FO found %d formats", nformats);

	// for a broken camera, we overwrite the number of formats
	if (device.getBroken() == BROKEN_THE_IMAGING_SOURCE) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "FO overwriting nformats");
		hd->setBNumFormats(nformats);
	}

	// return the header with all the formats attached
	debug(LOG_DEBUG, DEBUG_LOG, 0, "header descriptor complete");
	return USBDescriptorPtr(hd);
}

/**
 * \brief Parse frame descriptors from a data block
 *
 * \param data		data block containing the format descriptor
 * \param length	length of the data block
 * \param fd		format descriptor to attach the frame descriptors
 */
USBDescriptorPtr	VideoStreamingDescriptorFactory::format(
				const void *data, int length,
				FormatDescriptor *fd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FO completing a format descriptor, "
		"length = %d", length);

	// get offset to the first frame descriptor
	int	offset = fd->bLength();

	// the loop below iterates through the rest of the descriptor data
	// until what remains is not a frame descriptor
	int	nframes = 0;
	do {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "FR try at offset %d", offset);
		// parse the next header
		USBDescriptorPtr	newframe
			= descriptor(offset + (uint8_t *)data, length - offset);

		// verify that it is a Frame descriptor
		FrameDescriptor	*framed
			= dynamic_cast<FrameDescriptor *>(&*newframe);
		if (NULL == framed) {
			// it is not a frame descriptor, so we go to the
			// cleanup portion
			debug(LOG_DEBUG, DEBUG_LOG, 0, "FR not a frame descriptor");
			goto cleanup;
		} else {
			// this is a frame descriptor, add it th the format
			// descriptor and go to the next descriptor
			fd->frames.push_back(newframe);
			offset += newframe->bLength();
			nframes++;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "FR found a new frame");
		}

		// is there more space?
	} while (offset < length);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FR total format length: %d", offset);

cleanup:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FR %d frames found", nframes);
	// fix broken TIS cameras
	if (device.getBroken() == BROKEN_THE_IMAGING_SOURCE) {
		fd->setBNumFrameDescriptors(nframes);
	}

	// return the format with all the frames attached
	debug(LOG_DEBUG, DEBUG_LOG, 0, "FO format descriptor complete");
	return USBDescriptorPtr(fd);
}

/**
 * \brief Parse a single descriptor.
 *
 * This method parses descriptors of known types. Format descriptors also
 * get the Frame descriptors attached, Header descriptors get the
 * Format descriptors.
 * \param data		data block to use for parsing
 * \param length	length of the data block
 */
USBDescriptorPtr	VideoStreamingDescriptorFactory::descriptor(
	const void *data, int length)
		throw(std::length_error, UnknownDescriptorError) {

	// if there is not enough data even to find out what type of
	// descriptor comes next, then this is an error
	if (length < 2) {
		throw std::length_error("not engouth data for descriptor");
	}

	// check that there is enough data to process
	if (blength(data) > length) {
		throw std::length_error("not enough data for descriptor");
	}

	// ensure that we are parseing an interface descriptor
	uint8_t	type = bdescriptortype(data);
	if (type != CS_INTERFACE) {
		throw std::runtime_error("not in an class interface descriptor");
	}

	// check the subtype
	uint8_t	subtype = bdescriptorsubtype(data);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "descriptor subtype: %02x", subtype);

	USBDescriptorPtr	result;
	switch (subtype) {
	case VS_INPUT_HEADER:
		result = header(data, length,
			new InputHeaderDescriptor(device, data, length));
		break;
	case VS_OUTPUT_HEADER:
		result = header(data, length,
			new OutputHeaderDescriptor(device, data, length));
		break;
	case VS_FORMAT_UNCOMPRESSED:
		result = format(data, length,
			new FormatUncompressedDescriptor(device, data, length));
		break;
	case VS_FRAME_UNCOMPRESSED:
		result = USBDescriptorPtr(
			new FrameUncompressedDescriptor(device,
				data, length));
		break;
	case VS_FORMAT_MJPEG:
		result = format(data, length,
			new FormatMJPEGDescriptor(device, data, length));
		break;
	case VS_FRAME_MJPEG:
		result = USBDescriptorPtr(
			new FrameMJPEGDescriptor(device,
				data, length));
		break;
	case VS_FORMAT_FRAME_BASED:
		result = format(data, length,
			new FormatFrameBasedDescriptor(device, data, length));
		break;
	case VS_FRAME_FRAME_BASED:
		result = USBDescriptorPtr(
			new FrameFrameBasedDescriptor(device,
				data, length));
		break;
	case VS_STILL_IMAGE_FRAME:
	case VS_COLORFORMAT:
		// XXX should add real implementation of these two descriptors
		result = USBDescriptorPtr(new USBDescriptor(device, data, length));
		break;
	default:
		throw UnknownDescriptorError(type, subtype);
	}

	return result;
}

} // namespace uvc
} // namespace usb
} // namespace astro
