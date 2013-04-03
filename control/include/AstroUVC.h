/*
 * AstroUVC.h -- USB Video Class interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroUVC_h
#define _AstroUVC_h

#include <AstroUSB.h>
#include <tr1/memory>
#include <stdexcept>

#define	CC_VIDEO			0x0e

#define SC_UNDEFINED			0x00
#define SC_VIDECONTROL			0x01
#define SC_VIDEOSTREAMING		0x02
#define SC_VIDEO_INTERFACE_COLLECTION	0x03

#define	PC_PROTOCOL_UNDEFINED		0x00

#define CS_UNDEFINED			0x20
#define CS_DEVICE			0x21
#define CS_CONFIGURATION		0x22
#define CS_STRING			0x23
#define CS_INTERFACE			0x24
#define CS_ENDPOINT			0x25

#define VC_DESCRIPTOR_UNDEFINED		0x00
#define VC_HEADER			0x01
#define VC_INPUT_TERMINAL		0x02
#define VC_OUTPUT_TERMINAL		0x03
#define VC_SELECTOR_UNIT		0x04
#define VC_PROCESSING_UNIT		0x05
#define VC_EXTENSION_UNIT		0x06

#define	VS_UNDEFINED			0x00
#define VS_INPUT_HEADER			0x01
#define VS_OUTPUT_HEADER		0x02
#define VS_STILL_IMAGE_FRAME		0x03
#define VS_FORMAT_UNCOMPRESSED		0x04
#define VS_FRAME_UNCOMPRESSED		0x05
#define VS_FORMAT_MJPEG			0x06
#define VS_FRAME_MJPEG			0x07
#define VS_FORMAT_MPEG2TS		0x0a
#define VS_FORMAT_DV			0x0c
#define VS_COLORFORMAT			0x0d
#define VS_FORMAT_FRAME_BASED		0x10
#define VS_FRAME_FRAME_BASED		0x11
#define VS_FORMAT_STREAM_BASED		0x12

#define EP_UNDEFINED			0x00
#define EP_GENERAL			0x01
#define EP_ENDPOINT			0x02
#define EP_INTERRUPT			0x03

#define RC_UNDEFINED			0x00
#define SET_CUR				0x01
#define GET_CUR				0x81
#define GET_MIN				0x82
#define GET_MAX				0x83
#define GET_RES				0x84
#define GET_LEN				0x85
#define GET_INFO			0x86
#define GET_DEF				0x87

#define VC_CONTROL_UNDEFINED		0x00
#define VC_VIDEO_POWER_MODE_CONTROL	0x01
#define VC_REQUEST_ERROR_CODE_CONTROL	0x02

#define	TE_CONTROL_UNDEFINED		0x00

#define SU_CONTROL_UNDEFINED		0x00
#define SU_INPUT_SELECT_CONTROL		0x01

#define CT_CONTROL_UNDEFINED			0x00
#define CT_SCANNING_MODE_CONTROL		0x01
#define CT_AE_MODE_CONTROL			0x02
#define CT_AE_PRIOERITY_CONTROL			0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL	0x04
#define CT_EXPOSURE_TIME_REALTIVE_CONTROL	0x05
#define CT_FOCUS_ABSOLUTE_CONTROL		0x06
#define CT_FOCUS_RELATIVE_CONTROL		0x07
#define CT_FOCUS_AUTO_CONTROL			0x08
#define CT_IRIS_ABSOLUTE_CONTROL		0x09
#define CT_IRIS_RELATIVE_CONTROL		0x0a
#define CT_ZOOM_ABSOLUTE_CONTROL		0x0b
#define CT_ZOOM_RELATIVE_CONTROL		0x0c
#define CT_PANTILT_ABSOLUTE_CONTROL		0x0d
#define CT_PANTILT_RELATIVE_CONTROL		0x0e
#define CT_ROLL_ABSOLUTE_CONTROL		0x0f
#define CT_ROLL_RELATIVE_CONTROL		0x10
#define CT_PRIVACY_CONTROL			0x11

#define PU_CONTROL_UNDEFINED				0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL		0x01
#define PU_BRIGHTNESS_CONTROL				0x02
#define PU_CONTRAST_CONTROL				0x03
#define PU_GAIN_CONTROL					0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL			0x05
#define PU_HUE_CONTROL					0x06
#define PU_SATURATION_CONTROL				0x07
#define PU_SHARPNESS_CONTROL				0x08
#define PU_GAMMA_CONTROL				0x09
#define PU_WHITE_BALANCE_TEMPERATUR_CONTROL		0x0a
#define PU_WHITE_BALANCE_TEMPERATUR_AUTO_CONTROL	0x0b
#define PU_WHITE_BALANCE_COMPONENT_CONTROL		0x0c
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL		0x0d
#define PU_DIGITAL_MULTIPLIER_CONTROL			0x0e
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL		0x0f
#define PU_HUE_AUTO_CONTROL				0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL		0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL			0x12

#define XU_CONTROL_UNDEFINED		0x00

#define VS_CONTROL_UNDEFINED		0x00
#define VS_PROBE_CONTROL		0x01
#define VS_COMMIT_CONTROL		0x02
#define VS_STILL_PROBE_CONTROL		0x03
#define VS_STILL_COMMIT_CONTROL		0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL	0x05
#define VS_STREAM_ERROR_CODE_CONTROL	0x06
#define VS_GENERATE_KEY_FRAME_CONTROL	0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL	0x08
#define VS_SYNCH_DELAY_CONTROL		0x09

#define TT_VENDOR_SPECIFIC		0x0100
#define TT_STREAMING			0x0101

#define ITT_VENDOR_SPECIFIC		0x0200
#define ITT_CAMERA			0x0201
#define ITT_MEDIA_TRANSPORT_INPUT	0x0202

#define OTT_VENDOR_SPECIFIC		0x0300
#define OTT_DISPLAY			0x0301
#define OTT_MEDIA_TRANSPORT_OUTPUT	0x0302

#define EXTERNAL_VENDOR_SPECIFIC	0x0400
#define COMPOSITE_CONNECTOR		0x0401
#define SVIDEO_CONNECTOR		0x0402
#define COMPONENT_CONNECTOR		0x0403

namespace astro {
namespace usb {
namespace uvc {

/**
 * \brief Base class for all UVC descriptors
 */
class UVCDescriptor : public astro::usb::USBDescriptor {
	uint8_t	bdescriptorsubtype;
public:
	UVCDescriptor(const astro::usb::Device& device, const void *data, int length);
	UVCDescriptor(const UVCDescriptor& other);
	UVCDescriptor&	operator=(const UVCDescriptor& other);
	uint8_t	bDescriptorSubtype() const;
};

typedef	std::tr1::shared_ptr<UVCDescriptor>	UVCDescriptorPtr;

/**
 * \brief Factory class to produce UVC Descriptors from raw data
 */
class UVCDescriptorFactory : public DescriptorFactory {
protected:
	uint8_t	bdescriptorsubtype(const void *data) const;
public:
	UVCDescriptorFactory(const Device& device);
	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);
	using DescriptorFactory::descriptor; // unhide
};

/**
 * \brief Video Control Descriptor Factory
 */
class	HeaderDescriptor;
class	FormatDescriptor;
class VideoControlDescriptorFactory : public UVCDescriptorFactory {
protected:
	uint16_t	wterminaltype(const void *data) const;
	USBDescriptorPtr	header(const void *data, int length);
public:
	VideoControlDescriptorFactory(const Device& device);
	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);
	using DescriptorFactory::descriptor; // unhide
};

/**
 * \brief Video Streaming Descriptor Factory
 */
class VideoStreamingDescriptorFactory : public UVCDescriptorFactory {
	USBDescriptorPtr	header(const void *data, int length,
					HeaderDescriptor *headerdescriptor);
	USBDescriptorPtr	formats(const void *data, int length,
					FormatDescriptor *formatdescriptor);
public:
	VideoStreamingDescriptorFactory(const Device& device);
	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);
	using DescriptorFactory::descriptor; // unhide
};

/**
 * \brief Video Control Interface Header Descriptor
 */
class InterfaceHeaderDescriptor : public UVCDescriptor {
	std::vector<USBDescriptorPtr>	units;
	uint8_t	camera_terminal_id;
	uint32_t	camera_controls;
	uint8_t	processing_unit_id;
	uint32_t	processing_unit_controls;
	void	getIds();
public:
	InterfaceHeaderDescriptor(const Device& device,
		const void *data, int length);
	InterfaceHeaderDescriptor(const InterfaceHeaderDescriptor& other);
	InterfaceHeaderDescriptor&	operator=(const InterfaceHeaderDescriptor& other);
	~InterfaceHeaderDescriptor();

	uint16_t	bcdUVC() const;
	uint16_t	wTotalLength() const;
	uint32_t	dwClockFrequency() const;
	uint8_t		bInCollection() const;
	uint8_t		baInterface(int index) const throw(std::range_error);
	virtual	std::string	toString() const;

	// access to the units
	int	numUnits() const;
	const USBDescriptorPtr&	operator[](int index) const;
	friend class VideoControlDescriptorFactory;

	uint8_t	cameraTerminalID() const;
	uint32_t	cameraControls() const;
	uint8_t	processingUnitID() const;
	uint32_t	processingUnitControls() const;
};

bool	isInterfaceHeaderDescriptor(const USBDescriptorPtr& ptr);
InterfaceHeaderDescriptor	*interfaceHeaderDescriptor(USBDescriptorPtr& ptr);
const InterfaceHeaderDescriptor	*interfaceHeaderDescriptor(
	const USBDescriptorPtr& ptr);

/**
 * \brief Terminal Descriptor
 */
class TerminalDescriptor : public UVCDescriptor {
public:
	TerminalDescriptor(const Device& device,
		const void *data, int length);
	TerminalDescriptor(const TerminalDescriptor& other);
	TerminalDescriptor&	operator=(const TerminalDescriptor& other);

	uint8_t		bTerminalID() const;
	uint16_t	wTerminalType() const;
	uint8_t		bAssocTerminal() const;
	virtual std::string	toString() const;
};


/**
 * \brief Input Terminal Descriptor
 */
class InputTerminalDescriptor : public TerminalDescriptor {
	std::string	terminal;
public:
	InputTerminalDescriptor(const Device& device,
		const void *data, int length);
	InputTerminalDescriptor(const InputTerminalDescriptor& other);
	InputTerminalDescriptor&	operator=(const InputTerminalDescriptor& other);

	const std::string&	iTerminal() const;
	virtual std::string	toString() const;
};

/**
 * \brief Output Terminal Descriptor
 */
class OutputTerminalDescriptor : public TerminalDescriptor {
	std::string	terminal;
public:
	OutputTerminalDescriptor(const Device& device,
		const void *data, int length);
	OutputTerminalDescriptor(const OutputTerminalDescriptor& other);
	OutputTerminalDescriptor&	operator=(const OutputTerminalDescriptor& other);

	uint8_t		bSourceID() const;
	const std::string&	iTerminal() const;
	virtual std::string	toString() const;
};

/**
 * \brief Camera Terminal Descriptor
 */
class CameraTerminalDescriptor : public TerminalDescriptor {
	std::string	terminal;
public:
	CameraTerminalDescriptor(const Device& device,
		const void *data, int length);
	CameraTerminalDescriptor(const CameraTerminalDescriptor& other);
	CameraTerminalDescriptor&	operator=(const CameraTerminalDescriptor& other);

	const std::string&	iTerminal() const;
	uint16_t	wObjectiveFocalLengthMin() const;
	uint16_t	wObjectiveFocalLengthMax() const;
	uint16_t	wOcularFocalLength() const;
	uint8_t	bControlSize() const;
	uint32_t	bmControls() const;
	virtual std::string	toString() const;
};

bool	isCameraTerminalDescriptor(const USBDescriptorPtr& ptr);
CameraTerminalDescriptor	*cameraTerminalDescriptor(USBDescriptorPtr& ptr);

/**
 * \brief Selector Unit Descriptor
 */
class SelectorUnitDescriptor : public UVCDescriptor {
	std::string	selector;
public:
	SelectorUnitDescriptor(const Device& device,
		const void *data, int length);
	SelectorUnitDescriptor(const SelectorUnitDescriptor& other);
	SelectorUnitDescriptor&	operator=(const SelectorUnitDescriptor& other);
	uint8_t	bUnitID() const;
	uint8_t	bNrInPins() const;
	uint8_t	baSourceID(int index) const;
	const std::string&	iSelector() const;
	virtual	std::string	toString() const;
};

/**
 * \brief Processing Unit Descriptor
 */
class ProcessingUnitDescriptor : public UVCDescriptor {
	std::string	processing;
public:
	ProcessingUnitDescriptor(const Device& device, const void *data, int length);
	ProcessingUnitDescriptor(const ProcessingUnitDescriptor& other);
	ProcessingUnitDescriptor&	operator=(const ProcessingUnitDescriptor& other);
	uint8_t	bUnitID() const;
	uint8_t	bSourceID() const;
	uint16_t	wMaxMultiplier() const;
	uint8_t	bControlSize() const;
	uint32_t	bmControls() const;
	uint32_t	bmVideoStandards() const;
	const std::string&	iProcessing() const;
	virtual std::string	toString() const;
};

bool	isProcessingUnitDescriptor(const USBDescriptorPtr& ptr);
ProcessingUnitDescriptor	*processingUnitDescriptor(USBDescriptorPtr& ptr);

/**
 * \brief Extension Unit Descriptor
 */
class ExtensionUnitDescriptor : public UVCDescriptor {
	std::string	extension;
	std::string	guid;
public:
	ExtensionUnitDescriptor(const Device& device, const void *data, int length);
	ExtensionUnitDescriptor(const ExtensionUnitDescriptor& other);
	ExtensionUnitDescriptor&	operator=(const ExtensionUnitDescriptor& other);
	uint8_t	bUnitID() const;
	const std::string&	guidExtensionCode() const;
	uint8_t	bNumControls() const;
	uint8_t	bNrInPins() const;
	uint8_t	baSourceID(int index) const;
	uint8_t	bControlSize() const;
	uint32_t	bmControls() const;
	const std::string&	iExtension() const;
	virtual std::string	toString() const;
};

/**
 * \brief Header Descriptor , base class for Input/Output Header Desriptor
 */
class HeaderDescriptor : public UVCDescriptor {
	std::vector<USBDescriptorPtr>	formats;
public:
	HeaderDescriptor(const Device& device, const void *data, int length);
	HeaderDescriptor(const HeaderDescriptor& other);
	HeaderDescriptor&	operator=(const HeaderDescriptor &other);
	uint8_t		bNumFormats() const;
	uint16_t	wTotalLength() const;
	uint8_t		bEndpointAddress() const;
	virtual std::string	toString() const;
	const USBDescriptorPtr	operator[](int formatindex) const;
	friend class VideoStreamingDescriptorFactory;
};

/**
 * \brief Input Header Descriptor
 */
class InputHeaderDescriptor : public HeaderDescriptor {
public:
	InputHeaderDescriptor(const Device& device, const void *data, int length);
	InputHeaderDescriptor(const InputHeaderDescriptor& other);
	InputHeaderDescriptor&	operator=(const InputHeaderDescriptor& other);
	uint8_t		bmInfo() const;
	uint8_t		bTerminalLink() const;
	uint8_t		bStillCaptureMethod() const;
	uint8_t		bTriggerSupport() const;
	uint8_t		bTriggerUsage() const;
	uint8_t		bControlSize() const;
	uint32_t	bmaControls(int index) const;
	virtual std::string	toString() const;
};

/**
 * \brief Output Header Descriptor
 */
class OutputHeaderDescriptor : public HeaderDescriptor {
public:
	OutputHeaderDescriptor(const Device& device,
		const void *data, int length);
	OutputHeaderDescriptor(const OutputHeaderDescriptor& other);
	OutputHeaderDescriptor&	operator=(const OutputHeaderDescriptor& other);
	uint8_t		bTerminalLink() const;
	uint8_t		bControlSize() const;
	uint32_t	bmaControls(int index) const;
	virtual std::string	toString() const;
};

/**
 * \brief Format Descriptor
 */
class FormatDescriptor : public UVCDescriptor {
	std::vector<USBDescriptorPtr>	frames;
protected:
	std::string	framesToString() const;
public:
	FormatDescriptor(const Device& device, const void *data, int length);
	FormatDescriptor(const FormatDescriptor& other);
	FormatDescriptor&	operator=(const FormatDescriptor& other);
	uint8_t	bFormatIndex() const;
	uint8_t	bNumFrameDescriptors() const;

	virtual uint8_t	bDefaultFrameIndex() const = 0;
	virtual uint8_t	bAspectRatioX() const = 0;
	virtual uint8_t	bAspectRatioY() const = 0;
	virtual uint32_t	bmInterlaceFlags() const = 0;
	virtual uint8_t	bCopyProtect() const = 0;
	virtual std::string	toString() const;

	// frame descriptors
	int	numFrames() const;
	int	wTotalLength() const;	// not in the header, but needed
	const USBDescriptorPtr&	operator[](int frameindex) const;
	friend class VideoStreamingDescriptorFactory;
};

typedef std::tr1::shared_ptr<FormatDescriptor *>	FormatDescriptorPtr;

/**
 * \brief MJPEG Format Descriptor
 */
class FormatMJPEGDescriptor : public FormatDescriptor {
public:
	FormatMJPEGDescriptor(const Device& device, const void *data, int length);
	FormatMJPEGDescriptor(const FormatMJPEGDescriptor& other);
	FormatMJPEGDescriptor&	operator=(const FormatMJPEGDescriptor& other);
	virtual uint8_t	bDefaultFrameIndex() const;
	virtual uint8_t	bAspectRatioX() const;
	virtual uint8_t	bAspectRatioY() const;
	virtual uint32_t	bmInterlaceFlags() const;
	virtual uint8_t	bCopyProtect() const;
	virtual std::string	toString() const;
};

/**
 * \brief Frame Based Format Descriptor
 */
class FormatFrameBasedDescriptor : public FormatDescriptor {
public:
	FormatFrameBasedDescriptor(const Device& device,
		const void *data, int length);
	FormatFrameBasedDescriptor(const FormatFrameBasedDescriptor& other);
	FormatFrameBasedDescriptor&	operator=(
		const FormatFrameBasedDescriptor& other);
	uint8_t	bBitsPerPixel() const;
	virtual uint8_t	bDefaultFrameIndex() const;
	virtual uint8_t	bAspectRatioX() const;
	virtual uint8_t	bAspectRatioY() const;
	virtual uint32_t	bmInterlaceFlags() const;
	virtual uint8_t	bCopyProtect() const;
	std::string	guidFormat() const;
	virtual std::string	toString() const;
};

/**
 * \brief Uncompressed Format Descriptor
 */
class FormatUncompressedDescriptor : public FormatFrameBasedDescriptor {
public:
	FormatUncompressedDescriptor(const Device& device,
		const void *data, int length);
	FormatUncompressedDescriptor(const FormatUncompressedDescriptor& other);
	FormatUncompressedDescriptor&	operator=(
		const FormatUncompressedDescriptor& other);
	virtual std::string	toString() const;
};

/**
 * \brief Frame Descriptor base class
 */
class FrameDescriptor : public UVCDescriptor {
public:
	FrameDescriptor(const Device& device, const void *data, int length);
	FrameDescriptor(const FrameDescriptor& other);
	FrameDescriptor&	operator=(const FrameDescriptor& other);

	// common attributes for all Frame descriptors
	uint8_t		bFrameIndex() const;
	uint32_t	bmCapabilities() const;
	uint16_t	wWidth() const;
	uint16_t	wHeight() const;
	uint32_t	dwMinBitRate() const;
	uint32_t	dwMaxBitRate() const;

	virtual	uint8_t		bFrameIntervalType() const;
	virtual uint32_t	dwDefaultFrameInterval() const;
	// accessors for continuous frame intervals
	virtual uint32_t	dwMinFrameInterval() const;
	virtual uint32_t	dwMaxFrameInterval() const;
	virtual uint32_t	dwFrameIntervalStep() const;
	// accessors for discrete frame intervals
	virtual uint32_t	dwFrameInterval(int interval) const;

	virtual	std::string	toString() const;
};

/**
 * \brief FrameUncompressedDescriptor
 */
class FrameUncompressedDescriptor : public FrameDescriptor {
public:
	FrameUncompressedDescriptor(const Device& device,
		const void *data, int length);
	FrameUncompressedDescriptor(const FrameUncompressedDescriptor& other);
	FrameUncompressedDescriptor&	operator=(
		const FrameUncompressedDescriptor& other);

	uint32_t	dwMaxVideoFrameBufferSize() const;
	virtual std::string	toString() const;
};

/**
 * \brief FrameMJPEGDescriptor
 */
class FrameMJPEGDescriptor : public FrameDescriptor {
public:
	FrameMJPEGDescriptor(const Device& other, const void *data, int length);
	FrameMJPEGDescriptor(const FrameMJPEGDescriptor& other);
	FrameMJPEGDescriptor&	operator=(const FrameMJPEGDescriptor& other);

	uint32_t	dwMaxVideoFrameBufferSize() const;
	virtual std::string	toString() const;
};

/**
 * \brief FrameFrameBasedDescriptor
 */
class FrameFrameBasedDescriptor : public FrameDescriptor {
public:
	FrameFrameBasedDescriptor(const Device& device,
		const void *data, int length);
	FrameFrameBasedDescriptor(const FrameFrameBasedDescriptor& other);
	FrameFrameBasedDescriptor&	operator=(
		const FrameFrameBasedDescriptor& other);

	virtual	uint8_t		bFrameIntervalType() const;
	virtual uint32_t	dwDefaultFrameInterval() const;
	uint32_t	dwBytesPerLine() const;
	virtual std::string	toString() const;
};

/**
 * \brief Frame holder class
 */
class Frame : public std::string {
	int	width;
	int	height;
public:
	Frame(const void *data, int length);
	int	getWidth() const;
	int	getHeight() const;
	friend class UVCCamera;
};

/**
 * \brief UVC Camera
 */
class UVCCamera {
	// the device
	Device	device;
	DeviceHandle	*devicehandle;

	// interface association descriptor
	USBDescriptorPtr	iadptr;
	const InterfaceAssociationDescriptor&	iad() const;

	// stuff related to the video control interface
	USBDescriptorPtr	videocontrol;

	// a list of Video Streaming Interface descriptors, one for each video
	// streaming interface
	std::vector<USBDescriptorPtr>	videostreaming;
public:
	UVCCamera(const Device& device, bool force = false);
	~UVCCamera();

	// accessors for the control interface parameters
	uint8_t	controlInterface() const;
	uint8_t	controlCameraTerminalID() const;
	uint32_t	controlCameraControls() const;
	uint8_t	controlProcessingUnitID() const;
	uint32_t	controlProcessingUnitControls() const;

	// selecting format and frame
	USBDescriptorPtr	getFormat(uint8_t interface);
	void	setFormat(int formatindex);
	USBDescriptorPtr	getFrame(uint8_t interface);
	void	setFrame(int frameindex);

	// modifying parameter of the various interfaces

	// display stuff
	std::string	toString() const;

	// access to frames
#if 0
	Frame	getFrame();
	std::vector<Frame>	getFrames(int nframes);
#endif
};

std::ostream&	operator<<(std::ostream& out, const UVCCamera& camera);

/**
 * \brief data structure for video streaming interface control requests
 */
typedef struct  vs_control_request_s {
	uint16_t	bmHint;
	uint8_t		bFormatIndex;
	uint8_t		bFrameIndex;
	uint32_t	dwFrameInterval;
	uint16_t	wKeyFrameRate;
	uint16_t	wPFrameRate;
	uint16_t	wCompQuality;
	uint16_t	wCompWindowSize;
	uint16_t	wDelay;
	uint32_t	dwMaxVideoFrameSize;
	uint32_t	dwMaxPayloadTransferSize;
	uint32_t	dwClockFrequency;
	uint8_t		bmFrameingInfo;
	uint8_t		bPreferedVersion;
	uint8_t		bMinVersion;
	uint8_t		bMaxVersion;
} __attribute__((packed)) vs_control_request_t;

} // namespace uvc
} // namespace usb
} // namespace astro

#endif /* _AstroUVC_h */
