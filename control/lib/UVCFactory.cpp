/*
 * UVCFactory.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschuel Rappersil
 */
#include <AstroUVC.h>
#include <sstream>

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
std::cerr << "header parsed" << std::endl;

	// add the units
	int	offset = ifhd->bLength();
std::cerr << "offset: " << offset << ", length = " << length << std::endl;
	while (offset < length) {
		USBDescriptorPtr	unit
			= descriptor(offset + (uint8_t *)data, length - offset);
std::cerr << "new unit" << std::endl << unit;
		ifhd->units.push_back(unit);
		offset += unit->bLength();
	}

	// make sure we know about the camera and the processing unit
	// controls
std::cerr << "getting ids" << std::endl;
	ifhd->getIds();

std::cerr << "complete" << std::endl;
	// return the InterfaceHeader
	return USBDescriptorPtr(ifhd);
}

/**
 * \brief Main vide control descriptor parser function
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
std::cerr << "got a header" << std::endl;
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
std::cerr << "parse complete" << std::endl;
	return result;
}

//////////////////////////////////////////////////////////////////////
// VideoStreamingDescriptorFactory
//////////////////////////////////////////////////////////////////////
VideoStreamingDescriptorFactory::VideoStreamingDescriptorFactory(
	Device& _device)
	: UVCDescriptorFactory(_device) {
}

USBDescriptorPtr	VideoStreamingDescriptorFactory::header(
				const void *data, int length,
				HeaderDescriptor *hd) {
	// add the formats
	int	offset = hd->bLength();
	int	nformats = hd->bNumFormats();
	for (int formatindex = 0; formatindex < nformats; formatindex++) {
		USBDescriptorPtr	newformat
			= descriptor(offset + (uint8_t *)data, length - offset);
		hd->formats.push_back(newformat);

		FormatDescriptor	*fd
			= dynamic_cast<FormatDescriptor *>(&*newformat);
		if (NULL == fd) {
			throw std::runtime_error("expected a FormatDescriptor");
		}

		// find out how long that format is
		offset += fd->wTotalLength();

		// it is possible that there are still image frame descriptors
		// or color matching descriptors following the frames, so
		// we should skip them and add to the total length of the
		// format descriptor
		do {
			// get the next descriptor
			newformat = descriptor(offset + (uint8_t *)data,
				length - offset);

			// if it is a format descriptor, then we have seen 
			// all descriptors, and should leave the loop
			fd = dynamic_cast<FormatDescriptor *>(&*newformat);

			// if it is not a format descriptor, then we should
			// add it to the total length, and go to the next
			// descriptor
			if (fd == NULL) {
				offset += newformat->bLength();
			}
		} while ((fd == NULL) && ((length - offset) > 0));
	}

	// return the header with all the formats attached
	return USBDescriptorPtr(hd);
}

USBDescriptorPtr	VideoStreamingDescriptorFactory::formats(
				const void *data, int length,
				FormatDescriptor *fd) {
	int	offset = fd->bLength();
	int	nframes = fd->bNumFrameDescriptors();

	// get all the frame descriptors
	for (int frameindex = 0; frameindex < nframes; frameindex++) {
		USBDescriptorPtr	newframe
			= descriptor(offset + (uint8_t *)data, length - offset);
		fd->frames.push_back(newframe);
		offset += newframe->bLength();
	}
	std::cout << "total length of format: " << offset << std::endl;

	// return the format with all the frames attached
	return USBDescriptorPtr(fd);
}

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
		result = formats(data, length,
			new FormatUncompressedDescriptor(device, data, length));
		break;
	case VS_FRAME_UNCOMPRESSED:
		result = USBDescriptorPtr(
			new FrameUncompressedDescriptor(device,
				data, length));
		break;
	case VS_FORMAT_MJPEG:
		result = formats(data, length,
			new FormatMJPEGDescriptor(device, data, length));
		break;
	case VS_FRAME_MJPEG:
		result = USBDescriptorPtr(
			new FrameMJPEGDescriptor(device,
				data, length));
		break;
	case VS_FORMAT_FRAME_BASED:
		result = formats(data, length,
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
