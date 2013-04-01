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
UVCDescriptorFactory::UVCDescriptorFactory(const Device& _device)
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
	uint8_t	type = bdescriptortype(data);
	uint8_t	subtype = bdescriptorsubtype(data);
	std::cerr << "type = " << (int)type << ", subtype = "
		<< (int)subtype << std::endl;

	return DescriptorFactory::descriptor(data, length);
}

//////////////////////////////////////////////////////////////////////
// VideoControlDescriptorFactory
//////////////////////////////////////////////////////////////////////
VideoControlDescriptorFactory::VideoControlDescriptorFactory(
	const Device& _device)
	: UVCDescriptorFactory(_device) {
}

uint16_t	VideoControlDescriptorFactory::wterminaltype(const void *data)
			const {
	return *(uint16_t *)&(((uint8_t *)data)[4]);
}

USBDescriptorPtr	VideoControlDescriptorFactory::header(
				const void *data, int length) {
	std::cerr << "parsing video control interface descriptors" << std::endl;
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
	uint8_t	type = bdescriptortype(data);
	uint8_t	subtype = bdescriptorsubtype(data);
	std::cerr << "type = " << (int)type << ", subtype = "
		<< (int)subtype << std::endl;

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
	const Device& _device)
	: UVCDescriptorFactory(_device) {
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
	uint8_t	type = bdescriptortype(data);
	uint8_t	subtype = bdescriptorsubtype(data);

	USBDescriptorPtr	result;
	switch (subtype) {
	case VS_INPUT_HEADER:
		result = USBDescriptorPtr(new InputHeaderDescriptor(
			device, data, length));
		break;
	case VS_OUTPUT_HEADER:
		result = USBDescriptorPtr(new OutputHeaderDescriptor(
			device, data, length));
		break;
	case VS_FORMAT_UNCOMPRESSED:
		result = USBDescriptorPtr(
			new FormatUncompressedDescriptor(device,
				data, length));
		break;
	case VS_FRAME_UNCOMPRESSED:
		result = USBDescriptorPtr(
			new FrameUncompressedDescriptor(device,
				data, length));
		break;
	case VS_FORMAT_MJPEG:
		result = USBDescriptorPtr(
			new FormatMJPEGDescriptor(device,
				data, length));
		break;
	case VS_FRAME_MJPEG:
		result = USBDescriptorPtr(
			new FrameMJPEGDescriptor(device,
				data, length));
		break;
	case VS_FORMAT_FRAME_BASED:
		result = USBDescriptorPtr(
			new FormatFrameBasedDescriptor(device,
				data, length));
		break;
	case VS_FRAME_FRAME_BASED:
		result = USBDescriptorPtr(
			new FrameFrameBasedDescriptor(device,
				data, length));
		break;
	default:
		throw UnknownDescriptorError(type, subtype);
	}
}

} // namespace uvc
} // namespace usb
} // namespace astro
