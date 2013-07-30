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
#include <vector>

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
#define CT_AE_PRIORITY_CONTROL			0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL	0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL	0x05
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
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL		0x0a
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL	0x0b
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
 * \brief find out whether a USB device is a UVC camera device.
 *
 * This method returns true if the device is a UVC device or can be 
 * opened as a UVC device like the "The Imaging Source" cameras.
 * \param device	USB device to test
 */
bool	isUVCDevice(Device& device);

/**
 * \brief Base class for all UVC descriptors
 *
 * UVC descriptors have a subtype field at offset 2
 */
class UVCDescriptor : public astro::usb::USBDescriptor {
	uint8_t	bdescriptorsubtype;
	UVCDescriptor(const UVCDescriptor& other);
public:
	UVCDescriptor(astro::usb::Device& device, const void *data, int length);
	uint8_t	bDescriptorSubtype() const;
};

typedef	std::tr1::shared_ptr<UVCDescriptor>	UVCDescriptorPtr;

/**
 * \brief Factory class to produce UVC Descriptors from raw data
 *
 * This factory just redirects to the USBDescriptorFactory because there are 
 * no UVC descriptors common to video streaming and video control interface.
 * To parse video control descriptors, use the VideoControlDescriptorFactory,
 * and likewise for video streaming descriptors, use the
 * VideoStreamingDescriptorFactory.
 */
class UVCDescriptorFactory : public DescriptorFactory {
	UVCDescriptorFactory(const UVCDescriptorFactory& other);
protected:
	uint8_t	bdescriptorsubtype(const void *data) const;
public:
	UVCDescriptorFactory(Device& device);
	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);
	using DescriptorFactory::descriptor; // unhide
};

/**
 * \brief Video Control Descriptor Factory
 *
 * Video control descriptors are mainly the header descriptors and the
 * terminal and the unit descriptors. When the factory parses a header,
 * it also includes all the terminals and the units int he header, so
 * that the can be easily accessed without reparsing descriptors.
 */
class	HeaderDescriptor;
class	FormatDescriptor;
class VideoControlDescriptorFactory : public UVCDescriptorFactory {
	VideoControlDescriptorFactory(const VideoControlDescriptorFactory& other);
protected:
	uint16_t	wterminaltype(const void *data) const;
	USBDescriptorPtr	header(const void *data, int length);
public:
	VideoControlDescriptorFactory(Device& device);
	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);
	using DescriptorFactory::descriptor; // unhide
};

/**
 * \brief Video Streaming Descriptor Factory
 *
 * The Video streaming descriptors are primarily the format and frame
 * descriptors. When the factory parses a header or a format, it also
 * includes the dependent descriptors, i.e. formats and frames in the case
 * of the header, or frames in the case of a format descriptor.
 * The factory also includes some workarounds for broken cameras like
 * the "The Imaging Source" cameras which pretend to be vendor specific
 * devices but in reality are UVC video class cameras.
 */
class VideoStreamingDescriptorFactory : public UVCDescriptorFactory {
	USBDescriptorPtr	header(const void *data, int length,
					HeaderDescriptor *headerdescriptor);
	USBDescriptorPtr	format(const void *data, int length,
					FormatDescriptor *formatdescriptor);
	VideoStreamingDescriptorFactory(const VideoStreamingDescriptorFactory& other);
public:
	VideoStreamingDescriptorFactory(Device& device);
	virtual USBDescriptorPtr	descriptor(const void *data, int length)
		throw(std::length_error, UnknownDescriptorError);
	using DescriptorFactory::descriptor; // unhide
};

/**
 * \brief Video Control Interface Header Descriptor
 */
class InterfaceHeaderDescriptor : public UVCDescriptor {
	std::vector<USBDescriptorPtr>	units;
	// camera terminal and processing unit id and controls, this is
	// all we need to formulate control requests.
	uint8_t	camera_terminal_id;
	uint32_t	camera_controls;
	uint8_t	processing_unit_id;
	uint32_t	processing_unit_controls;
	void	getIds();
	InterfaceHeaderDescriptor(const InterfaceHeaderDescriptor& other);
public:
	InterfaceHeaderDescriptor(Device& device,
		const void *data, int length);
	~InterfaceHeaderDescriptor();

	uint16_t	bcdUVC() const;
	uint16_t	wTotalLength() const;
	uint32_t	dwClockFrequency() const;
	uint8_t		bInCollection() const;
	uint8_t		baInterface(int index) const throw(std::range_error);
	virtual	std::string	toString() const;

	// access to the units
	int	numUnits() const;
	const USBDescriptorPtr&	operator[](size_t index) const;
	friend class VideoControlDescriptorFactory;

	// accessors for camera terminal and processing unit
	uint8_t	cameraTerminalID() const;
	uint32_t	cameraControls() const;
	uint8_t	processingUnitID() const;
	uint32_t	processingUnitControls() const;
};

/**
 * \brief Terminal Descriptor
 */
class TerminalDescriptor : public UVCDescriptor {
	TerminalDescriptor(const TerminalDescriptor& other);
public:
	TerminalDescriptor(Device& device,
		const void *data, int length);

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
	InputTerminalDescriptor(const InputTerminalDescriptor& other);
public:
	InputTerminalDescriptor(Device& device,
		const void *data, int length);

	const std::string&	iTerminal() const;
	virtual std::string	toString() const;
};

/**
 * \brief Output Terminal Descriptor
 */
class OutputTerminalDescriptor : public TerminalDescriptor {
	std::string	terminal;
	OutputTerminalDescriptor(const OutputTerminalDescriptor& other);
public:
	OutputTerminalDescriptor(Device& device,
		const void *data, int length);

	uint8_t		bSourceID() const;
	const std::string&	iTerminal() const;
	virtual std::string	toString() const;
};

/**
 * \brief Camera Terminal Descriptor
 */
class CameraTerminalDescriptor : public TerminalDescriptor {
	std::string	terminal;
	CameraTerminalDescriptor(const CameraTerminalDescriptor& other);
public:
	CameraTerminalDescriptor(Device& device,
		const void *data, int length);

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
	SelectorUnitDescriptor(const SelectorUnitDescriptor& other);
public:
	SelectorUnitDescriptor(Device& device,
		const void *data, int length);
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
	ProcessingUnitDescriptor(const ProcessingUnitDescriptor& other);
public:
	ProcessingUnitDescriptor(Device& device, const void *data, int length);
	uint8_t	bUnitID() const;
	uint8_t	bSourceID() const;
	uint16_t	wMaxMultiplier() const;
	uint8_t	bControlSize() const;
	uint32_t	bmControls() const;
	uint32_t	bmVideoStandards() const;
	const std::string&	iProcessing() const;
	virtual std::string	toString() const;
};

/**
 * \brief Extension Unit Descriptor
 */
class ExtensionUnitDescriptor : public UVCDescriptor {
	std::string	extension;
	std::string	guid;
	ExtensionUnitDescriptor(const ExtensionUnitDescriptor& other);
public:
	ExtensionUnitDescriptor(Device& device, const void *data, int length);
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
 * 
 * This is the base class for headers. Depending on whether an interface
 * is used for video input (the only case we are interested in) or output
 * (not used in our case), the first video streaming header descriptor
 * is either an InputHeaderDescriptor or an OutputHeaderDescriptor, both
 * derived from this class.
 */
class HeaderDescriptor : public UVCDescriptor {
	std::vector<USBDescriptorPtr>	formats;
	HeaderDescriptor(const HeaderDescriptor& other);
	// methods to modify total length and number of formats, needed
	// to work around bugs in certain cameras
	void	setBNumFormats(uint8_t b);
	void	setWTotalLength(uint16_t w);
public:
	HeaderDescriptor(Device& device, const void *data, int length);
	uint8_t		bNumFormats() const;
	uint16_t	wTotalLength() const;
	uint8_t		bEndpointAddress() const;
	virtual std::string	toString() const;
	const USBDescriptorPtr	operator[](size_t formatindex) const;
	friend class VideoStreamingDescriptorFactory;
};

/**
 * \brief Input Header Descriptor
 */
class InputHeaderDescriptor : public HeaderDescriptor {
	InputHeaderDescriptor(const InputHeaderDescriptor& other);
public:
	InputHeaderDescriptor(Device& device, const void *data, int length);
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
	OutputHeaderDescriptor(const OutputHeaderDescriptor& other);
public:
	OutputHeaderDescriptor(Device& device,
		const void *data, int length);
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
	FormatDescriptor(const FormatDescriptor& other);
	void	setBNumFrameDescriptors(uint8_t b);
protected:
	std::string	framesToString() const;
public:
	FormatDescriptor(Device& device, const void *data, int length);
	uint8_t	bFormatIndex() const;
	uint8_t	bNumFrameDescriptors() const;

	virtual uint8_t	bDefaultFrameIndex() const = 0;
	virtual uint8_t	bAspectRatioX() const = 0;
	virtual uint8_t	bAspectRatioY() const = 0;
	virtual uint32_t	bmInterlaceFlags() const = 0;
	virtual uint8_t	bCopyProtect() const = 0;
	virtual std::string	toString() const;

	// frame descriptors
	size_t	numFrames() const;
	int	wTotalLength() const;	// not in the header, but needed
	const USBDescriptorPtr&	operator[](size_t frameindex) const;
	friend class VideoStreamingDescriptorFactory;
};

typedef std::tr1::shared_ptr<FormatDescriptor *>	FormatDescriptorPtr;

/**
 * \brief MJPEG Format Descriptor
 */
class FormatMJPEGDescriptor : public FormatDescriptor {
	FormatMJPEGDescriptor(const FormatMJPEGDescriptor& other);
public:
	FormatMJPEGDescriptor(Device& device, const void *data, int length);
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
	FormatFrameBasedDescriptor(const FormatFrameBasedDescriptor& other);
public:
	FormatFrameBasedDescriptor(Device& device,
		const void *data, int length);

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
	FormatUncompressedDescriptor(const FormatUncompressedDescriptor& other);
public:
	FormatUncompressedDescriptor(Device& device,
		const void *data, int length);
	virtual std::string	toString() const;
};

/**
 * \brief Frame Descriptor base class
 */
class FrameDescriptor : public UVCDescriptor {
	FrameDescriptor(const FrameDescriptor& other);
public:
	FrameDescriptor(Device& device, const void *data, int length);

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

	// combined accessor for minimum frame interal
	virtual uint32_t	minFrameInterval() const;

	virtual	std::string	toString() const;
};

/**
 * \brief FrameUncompressedDescriptor
 */
class FrameUncompressedDescriptor : public FrameDescriptor {
	FrameUncompressedDescriptor(const FrameUncompressedDescriptor& other);
public:
	FrameUncompressedDescriptor(Device& device,
		const void *data, int length);

	uint32_t	dwMaxVideoFrameBufferSize() const;
	virtual std::string	toString() const;
};

/**
 * \brief FrameMJPEGDescriptor
 */
class FrameMJPEGDescriptor : public FrameDescriptor {
	FrameMJPEGDescriptor(const FrameMJPEGDescriptor& other);
public:
	FrameMJPEGDescriptor(Device& other, const void *data, int length);

	uint32_t	dwMaxVideoFrameBufferSize() const;
	virtual std::string	toString() const;
};

/**
 * \brief FrameFrameBasedDescriptor
 */
class FrameFrameBasedDescriptor : public FrameDescriptor {
	FrameFrameBasedDescriptor(const FrameFrameBasedDescriptor& other);
public:
	FrameFrameBasedDescriptor(Device& device,
		const void *data, int length);

	virtual	uint8_t		bFrameIntervalType() const;
	virtual uint32_t	dwDefaultFrameInterval() const;
	uint32_t	dwBytesPerLine() const;
	virtual std::string	toString() const;
};

/**
 * \brief Traits class for Camera Unit and processing unit requets
 */
struct video_control_tag { };
	struct device_power_mode_control_tag
		: public video_control_tag {
		typedef enum CS_enum {
			CS = VC_VIDEO_POWER_MODE_CONTROL
		} CS_type;
	};
	struct request_error_code_control_tag
		: public video_control_tag {
		typedef enum CS_enum {
			CS = VC_REQUEST_ERROR_CODE_CONTROL
		} CS_type;
	};

struct camera_terminal_control_tag {
};
	struct scanning_mode_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_SCANNING_MODE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 0
		} bit_type;
	};
	struct auto_exposure_mode_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_AE_MODE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 1
		} bit_type;
	};
	struct auto_exposure_priority_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_AE_PRIORITY_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 2
		} bit_type;
	};
	struct exposure_time_absolute_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_EXPOSURE_TIME_ABSOLUTE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 3
		} bit_type;
	};
	struct exposure_time_relative_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_EXPOSURE_TIME_RELATIVE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 4
		} bit_type;
	};
	struct focus_absolute_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_FOCUS_ABSOLUTE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 5
		} bit_type;
	};
	struct focus_relative_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_FOCUS_RELATIVE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 6
		} bit_type;
	};
	struct focus_auto_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_FOCUS_AUTO_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 17
		} bit_type;
	};
	struct iris_absolute_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_IRIS_ABSOLUTE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 7
		} bit_type;
	};
	struct iris_relative_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_IRIS_RELATIVE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 8
		} bit_type;
	};
	struct zoom_absolute_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_ZOOM_ABSOLUTE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 9
		} bit_type;
	};
	struct zoom_relative_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_ZOOM_RELATIVE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 10
		} bit_type;
	};
	struct pantilt_absolute_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_PANTILT_ABSOLUTE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 11
		} bit_type;
	};
	struct pantilt_relative_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_PANTILT_RELATIVE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 12
		} bit_type;
	};
	struct roll_absolute_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_ROLL_ABSOLUTE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 13
		} bit_type;
	};
	struct roll_relative_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_ROLL_RELATIVE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 14
		} bit_type;
	};
	struct privacy_shutter_control_tag
		: public camera_terminal_control_tag {
		typedef enum CS_enum {
			CS = CT_PRIVACY_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 18
		} bit_type;
	};

struct selection_unit_control_tag {
};
	struct input_selector_control_tag
		: public selection_unit_control_tag {
		typedef enum CS_enum {
			CS = SU_INPUT_SELECT_CONTROL
		} CS_type;
	};

struct processing_unit_control_tag {
};
	struct backlight_compensation_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_BACKLIGHT_COMPENSATION_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 8
		} bit_type;
	};
	struct brightness_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_BRIGHTNESS_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 0
		} bit_type;
	};
	struct contrast_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_CONTRAST_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 1
		} bit_type;
	};
	struct gain_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_GAIN_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 9
		} bit_type;
	};
	struct power_line_frequency_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_POWER_LINE_FREQUENCY_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 10
		} bit_type;
	};
	struct hue_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_HUE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 2
		} bit_type;
	};
	struct hue_auto_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_HUE_AUTO_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 11
		} bit_type;
	};
	struct saturation_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_SATURATION_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 3
		} bit_type;
	};
	struct sharpness_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_SHARPNESS_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 4
		} bit_type;
	};
	struct gamma_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_GAMMA_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 5
		} bit_type;
	};
	struct white_balance_temperature_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_WHITE_BALANCE_TEMPERATURE_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 6
		} bit_type;
	};
	struct white_balance_temperature_auto_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 12
		} bit_type;
	};
	struct white_balance_component_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_WHITE_BALANCE_COMPONENT_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 7
		} bit_type;
	};
	struct white_balance_component_auto_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 13
		} bit_type;
	};
	struct digital_multiplier_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_DIGITAL_MULTIPLIER_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 14
		} bit_type;
	};
	struct digital_multiplier_limit_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 15
		} bit_type;
	};
	struct analog_video_standard_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_ANALOG_VIDEO_STANDARD_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 16
		} bit_type;
	};
	struct analog_lock_status_control_tag
		: public processing_unit_control_tag  {
		typedef enum CS_enum {
			CS = PU_ANALOG_LOCK_STATUS_CONTROL
		} CS_type;
		typedef enum bit_enum {
			bit = 17
		} bit_type;
	};

/**
 * \brief structures for UVC get/set requests
 */
typedef struct device_power_mode_control_s {
	typedef device_power_mode_control_tag	control_type;
	uint8_t	bDevicePowerMode;
} __attribute__((packed)) device_power_mode_control_t;

typedef struct request_error_code_control_s {
	typedef request_error_code_control_tag	control_type;
	uint8_t	bRequestErrorCode;
} __attribute__((packed)) request_error_control_t;

typedef struct scanning_mode_control_s {
	typedef scanning_mode_control_tag	control_type;
	uint8_t bScanningMode;
} __attribute__((packed)) scanning_mode_control_t;

typedef struct auto_exposure_mode_control_s {
	typedef auto_exposure_mode_control_tag	control_type;
	uint8_t bAutoExposureMode;
} __attribute__((packed)) auto_exposure_mode_control_t;

typedef struct auto_exposure_priority_control_s {
	typedef auto_exposure_priority_control_tag	control_type;
	uint8_t bAutoExposurePriority;
} __attribute__((packed)) auto_exposure_priority_control_t;

typedef struct exposure_time_absolute_control_s {
	typedef exposure_time_absolute_control_tag	control_type;
	uint32_t dwExposureTimeAbsolute;
} __attribute__((packed)) exposure_time_absolute_control_t;

typedef struct exposure_time_relative_control_s {
	typedef exposure_time_relative_control_tag	control_type;
	uint32_t dwExposureTimeRelative;
} __attribute__((packed)) exposure_time_relative_control_t;

typedef struct focus_absolute_control_s {
	typedef focus_absolute_control_tag	control_type;
	uint16_t wFocusAbsolute;
} __attribute__((packed)) focus_absolute_control_t;

typedef struct focus_relative_control_s {
	typedef focus_relative_control_tag	control_type;
	uint8_t bFocusRelative;
	uint8_t	bSpeed;
} __attribute__((packed)) focus_relative_control_t;

typedef struct focus_auto_control_s {
	typedef focus_auto_control_tag	control_type;
	uint8_t	bFocusAuto;
} __attribute__((packed)) focus_auto_control_t;

typedef struct iris_absolute_control_s {
	typedef iris_absolute_control_tag	control_type;
	uint16_t	wIrisAbsolute;
} __attribute__((packed)) iris_absolute_control_t;

typedef struct iris_relative_control_s {
	typedef iris_relative_control_tag	control_type;
	uint16_t	wIrisRelative;
} __attribute__((packed)) iris_relative_control_t;

typedef struct zoom_absolute_control_s {
	typedef zoom_absolute_control_tag	control_type;
	uint16_t	wObjectFocalLength;
} __attribute__((packed)) zoom_absolute_control_t;

typedef struct zoom_relative_control_s {
	typedef zoom_relative_control_tag	control_type;
	uint8_t	bZoom;
	uint8_t	bDigitalZoom;
	uint8_t	bSpeed;
} __attribute__((packed)) zoom_relative_control_t;

typedef struct pantilt_absolute_control_s {
	typedef pantilt_absolute_control_tag	control_type;
	uint32_t	dwPanAbsolute;
	uint32_t	dwTiltAbsolute;
} __attribute__((packed)) pantilt_absolute_control_t;

typedef struct pantilt_relative_control_s {
	typedef pantilt_relative_control_tag	control_type;
	uint8_t	bPanRelative;
	uint8_t	bPanSpeed;
	uint8_t	bTiltRelative;
	uint8_t	bTiltSpeed;
} __attribute__((packed)) pantilt_relative_control_t;

typedef struct roll_absolute_control_s {
	typedef roll_absolute_control_tag	control_type;
	uint16_t	wRoolAbsolute;
} __attribute__((packed)) roll_absolute_control_t;

typedef struct roll_relative_control_s {
	typedef roll_relative_control_tag	control_type;
	uint8_t	bRollRelative;
	uint8_t	bSpeed;
} __attribute__((packed)) roll_relative_control_t;

typedef struct privacy_shutter_control_s {
	typedef privacy_shutter_control_tag	control_type;
	uint8_t	bPrivacy;
} __attribute__((packed)) privacy_shutter_control_t;

typedef struct input_selector_control_s {
	typedef input_selector_control_tag	control_type;
	uint8_t	bSelector;
} __attribute__((packed)) input_selector_control_t;

typedef struct backlight_compensation_control_s {
	typedef backlight_compensation_control_tag	control_type;
	uint16_t	wBacklightCompensation;
} __attribute__((packed)) backlight_compensation_control_t;

typedef struct brightness_control_s {
	typedef brightness_control_tag control_type;
	uint16_t	wBrightness;
} __attribute__((packed)) brightness_control_t;

typedef struct contrast_control_s {
	typedef contrast_control_tag control_type;
	uint16_t	wContrast;
} __attribute__((packed)) contrast_control_t;

typedef struct gain_control_s {
	typedef gain_control_tag control_type;
	uint16_t	wGain;
} __attribute__((packed)) gain_control_t;

typedef struct power_line_frequency_control_s {
	typedef power_line_frequency_control_tag control_type;
	uint8_t	bPowerLineFrequency;
} __attribute__((packed)) power_line_frequency_control_t;

typedef struct hue_control_s {
	typedef hue_control_tag control_type;
	uint16_t	wHue;
} __attribute__((packed)) hue_control_t;

typedef struct hue_auto_control_s {
	typedef hue_auto_control_tag control_type;
	uint8_t	bHueAuto;
} __attribute__((packed)) hue_auto_control_t;

typedef struct saturation_control_s {
	typedef saturation_control_tag control_type;
	uint16_t	wSaturation;
} __attribute__((packed)) saturation_control_t;

typedef struct sharpness_control_s {
	typedef sharpness_control_tag control_type;
	uint16_t	wSharpness;
} __attribute__((packed)) sharpness_control_t;

typedef struct gamma_control_s {
	typedef gamma_control_tag control_type;
	uint16_t	wGamma;
} __attribute__((packed)) gamma_control_t;

typedef struct white_balance_temperature_control_s {
	typedef white_balance_temperature_control_tag control_type;
	uint16_t	wWhiteBalanceTemperature;
} __attribute__((packed)) white_balance_temperature_control_t;

typedef struct white_balance_temperature_auto_control_s {
	typedef white_balance_temperature_auto_control_tag control_type;
	uint8_t	bWhiteBalanceTemperatureAuto;
} __attribute__((packed)) white_balance_temperature_auto_control_t;

typedef struct white_balance_component_control_s {
	typedef white_balance_component_control_tag control_type;
	uint16_t	wWhiteBalanceBlue;
	uint16_t	wWhiteBalanceRed;
} __attribute__((packed)) white_balance_component_control_t;

typedef struct white_balance_component_auto_control_s {
	typedef white_balance_component_auto_control_tag control_type;
	uint8_t	bWhiteBalanceComponentAuto;
} __attribute__((packed)) white_balance_component_auto_control_t;

typedef struct digital_multiplier_control_s {
	typedef digital_multiplier_control_tag control_type;
	uint16_t	wMultiplierStep;
} __attribute__((packed)) digital_multiplier_control_t;

typedef struct digital_multiplier_limit_control_s {
	typedef digital_multiplier_limit_control_tag control_type;
	uint16_t	wMultiplierLimit;
} __attribute__((packed)) digital_multiplier_limit_control_t;

typedef struct analog_video_standard_control_s {
	typedef analog_video_standard_control_tag control_type;
	uint16_t	bVideoStandard;
} __attribute__((packed)) analog_video_standard_control_t;

typedef struct analog_lock_status_control_s {
	typedef analog_lock_status_control_tag control_type;
	uint16_t	wBrightness;
} __attribute__((packed)) analog_lock_status_control_t;


/**
 * \brief UVC Camera
 *
 * The USB Video Class Camera is a rather complicated object. An interface
 * association descriptor describes which interfaces belong to the video
 * function. The first interface of the association is the video control
 * interface. A pointer to this interface descriptor is kept in the 
 * videocontrol member variable.
 */
class UVCCamera {
	// the device
	Device&	device;

	/**
	 * interface association descriptor, which indicates which interface
	 * number points to the video control interface and which interfaces
	 * are video streaming interfaces
	 */
	USBDescriptorPtr	iadptr;
public:
	const InterfaceAssociationDescriptor&	iad() const;

private:
	/**
 	 * interface descriptor for the video control interface of the
	 * video function.
	 */
	// stuff related to the video control interface
	InterfacePtr	videocontrol;

	/**
 	 * \brief video control descriptors
	 *
 	 * This vector stores all video control descriptors. Many of them
	 * are not used, but the camera terminal and the process unit are
	 * needed to control the settings of the camera. They are made
	 * accessible separately in public methods below.
 	 */
	std::vector<USBDescriptorPtr>	videocontroldescriptors;

private:
	InterfaceHeaderDescriptor	*interfaceHeaderDescriptor() const;
	CameraTerminalDescriptor	*cameraTerminalDescriptor() const;
	ProcessingUnitDescriptor	*processingUnitDescriptor() const;
public:
	// accessors for the control interface parameters
	uint8_t		controlInterfaceNumber() const;
	uint8_t		controlCameraTerminalID() const;
	uint32_t	controlCameraControls() const;
	uint8_t		controlProcessingUnitID() const;
	uint32_t	controlProcessingUnitControls() const;

private:
	// find out whether a camera terminal supports a given control
	template<typename T>
	bool	controlSupported(const T& p, camera_terminal_control_tag) {
		uint32_t	bm = controlCameraControls();
		return (bm & (1 << T::control_type::bit)) ? true : false;
	}

	// find out whether a processing unit supports a given control
	template<typename T>
	bool	controlSupported(const T& p, processing_unit_control_tag) {
		uint32_t	bm = controlProcessingUnitControls();
		return (bm & (1 << T::control_type::bit)) ? true : false;
	}

public:
	/**
	 * \brief Find out whether a give control is supported
	 *
	 * This template function tests whether a given control is supported
	 * by the camera. Depending on the type of the control, according
	 * to the control_type structure, one of the private controlSupported
	 * methods is called. This implements compile time polymorphism for
	 * the controlSupported method.
 	 * \param p	structure of the control type to 
	 */
	template<typename T>
	bool	controlSupported(const T& p) {
		return controlSupported(p, typename T::control_type());
	}

	/**
	 * a list of Video Streaming Interface descriptors, one for each video
	 * streaming interface
	 */
	std::vector<USBDescriptorPtr>	videostreaming;

	/**
	 * \brief Currently selected frame width
 	 */
	int	width;

	/**
	 * \brief Currently selected frame height
 	 */
	int	height;

	/**
	 * \brief Currently selected bits per pixel
	 */
	int	bitsPerPixel;

	/**
	 * \brief Currently set frame interval
 	 */
	uint32_t	frameinterval;

	/**
	 * \brief 
	 */
	uint32_t	maxvideoframesize;

	/**
	 * \brief maximum payload transfer size
	 */
	uint32_t	maxpayloadtransfersize;
public:
	// constructors
	UVCCamera(Device& device, bool force = false) throw(USBError);
	~UVCCamera();

	// accessors to the video streaming interfaces
private:
	size_t		streamingInterfaceIndex(size_t interfacenumber) const
		throw(std::range_error);
	void	getCur(uint8_t interface);
public:
	size_t	numberVideoStreamingInterfaces() const;
	const USBDescriptorPtr&	operator[](size_t interfacenumber) const
		throw(std::range_error);
	USBDescriptorPtr&	operator[](size_t interfacenumber)
		throw(std::range_error);

	// access to format and frame descriptors
	USBDescriptorPtr	getHeaderDescriptor(uint8_t interfacenumber);
	USBDescriptorPtr	getFormatDescriptor(uint8_t interfacenumber,
		uint8_t formatindex);
	USBDescriptorPtr	getFrameDescriptor(uint8_t interfacenumber,
		uint8_t formatindex, uint8_t frameindex);

	// selecting format and frame
	uint32_t	minFrameInterval(uint8_t interfacenumber,
		uint8_t format, uint8_t frame) throw(std::range_error,USBError);
	void	selectFormatAndFrame(uint8_t interfacenumber,
			uint8_t format, uint8_t frame) throw(USBError);
	std::pair<uint8_t, uint8_t>	getFormatAndFrame(uint8_t interfacenumber)
		throw(USBError);

	// alternate setting selection for transfer
private:
	int	preferredAltSetting(uint8_t interface);

private:
	// private templates for the various types of requests
	// SET_CUR requests
	template<typename T>
	void	doSetCurrent(const T& p, video_control_tag) {
		uint16_t	wIndex = controlInterfaceNumber();
		Request<T>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			SET_CUR, T::control_type::CS << 8);
		device.controlRequest(&r);
	}

	template<typename T>
	void	doSetCurrent(const T& p, camera_terminal_control_tag) {
		uint16_t	wIndex = (controlCameraTerminalID() << 8)
						| controlInterfaceNumber();
		Request<T>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			SET_CUR, T::control_type::CS << 8, &p);
		device.controlRequest(&r);
	}

	template<typename T>
	void	doSetCurrent(const T& p, processing_unit_control_tag) {
		uint16_t	wIndex = (controlProcessingUnitID() << 8)
						| controlInterfaceNumber();
		Request<T>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			SET_CUR, T::control_type::CS << 8, &p);
		device.controlRequest(&r);
	}

public:
	// modifying parameter of the various interfaces
	template<typename T>
	void	setCurrent(const T& p) {
		doSetCurrent(p, typename T::control_type());
	}

	// GET_INFO request
	typedef struct control_info_s {
		uint8_t	info;
	} __attribute__((packed)) control_info_t;

	template<typename T>
	uint8_t	getInfo(T, video_control_tag) {
		uint16_t	wIndex = (controlCameraTerminalID() << 8)
						| controlInterfaceNumber();
		Request<control_info_t>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			(uint8_t)GET_INFO, (uint16_t)(T::control_type::CS << 8));
		device.controlRequest(&r);
		return r.data()->info;
	}

	template<typename T>
	uint8_t	getInfo(T, camera_terminal_control_tag) {
		uint16_t	wIndex = (controlCameraTerminalID() << 8)
						| controlInterfaceNumber();
		Request<control_info_t>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			(uint8_t)GET_INFO, (uint16_t)(T::control_type::CS << 8));
		device.controlRequest(&r);
		return r.data()->info;
	}

	template<typename T>
	uint8_t	getInfo(T, processing_unit_control_tag) {
		uint16_t	wIndex = (controlProcessingUnitID() << 8)
						| controlInterfaceNumber();
		Request<control_info_t>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			(uint8_t)GET_INFO, (uint16_t)(T::control_type::CS << 8));
		device.controlRequest(&r);
		return r.data()->info;
	}
public:
	template<typename T>
	uint8_t	getInfo(T) {
		return getInfo(T(), typename T::control_type());
	}

private:

	// GET_* requests
	template<typename T>
	T	doGetControl(int request, T, camera_terminal_control_tag) {
		uint16_t	wIndex = (controlCameraTerminalID() << 8)
						| controlInterfaceNumber();
		Request<T>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			request, T::control_type::CS << 8);
		device.controlRequest(&r);
		return *r.data();
	}

	template<typename T>
	T	doGetControl(int request, T, processing_unit_control_tag) {
		uint16_t	wIndex = (controlProcessingUnitID() << 8)
						| controlInterfaceNumber();
		Request<T>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			request, T::control_type::CS << 8);
		device.controlRequest(&r);
		return *r.data();
	}

	template<typename T>
	T	doGetControl(int request, T, video_control_tag) {
		uint16_t	wIndex = controlInterfaceNumber();
		Request<T>	r(RequestBase::class_specific_type,
			RequestBase::interface_recipient, wIndex,
			request, T::control_type::CS << 8);
		device.controlRequest(&r);
		return *r.data();
	}
	
public:
	template<typename T>
	T	get(int request, T) {
		return doGetControl(request, T(), typename T::control_type());
	}

	// some convenience set requests
	void	setExposureTime(double exposuretime);

	// gain control
	bool	hasGain();
	void	setGain(double gain);
	std::pair<float, float>	getGainInterval();

	// white balance control
	void	disableAutoWhiteBalance();

public:
	// display the camera
	std::string	toString() const;

	// access to frames
private:
	std::vector<FramePtr>	getIsoFrames(uint8_t interface,
					unsigned int nframes);
	std::vector<FramePtr>	getBulkFrames(uint8_t interface,
					unsigned int nframes);
public:
	FramePtr	getFrame(uint8_t interface);
	std::vector<FramePtr>	getFrames(uint8_t interface,
		unsigned int nframes);
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

/**
 * \brief Video Streaming Probe request.
 *
 * This request is used for the probe and commit protocol to negotiate the
 * video parameters.
 */
class VideoStreamingProbeControlRequest : public Request<vs_control_request_t> {
public:
	VideoStreamingProbeControlRequest(InterfacePtr interptr,
		uint8_t bRequest, vs_control_request_t *data = NULL);
};

/**
 * \brief Video Streaming commit request.
 *
 * This request is used for the probe and commit protocol to negotiate the
 * video parameters.
 */
class VideoStreamingCommitControlRequest : public Request<vs_control_request_t>{
public:
	VideoStreamingCommitControlRequest(InterfacePtr interptr,
		uint8_t bRequest, vs_control_request_t *data = NULL);
};

/**
 * \brief Payload packets for UVC transfers.
 *
 * UVC payload packets have a header containing information about which
 * packets belong together to form a frame. This class just implements
 * easy access to the header information.
 */
class UVCPayloadPacket {
	const std::string&	data;
public:
	UVCPayloadPacket(const std::string& data);
	uint8_t	hle() const;
	uint8_t	bfh() const;
	bool	eoh() const;
	bool	err() const;
	bool	sti() const;
	bool	res() const;
	bool	scr() const;
	bool	pts() const;
	bool	eof() const;
	bool	fid() const;
	uint32_t	ptsValue() const;
	uint64_t	scrValue() const;
	std::string	payload() const;
};

/**
 * \brief Bulk transfer for UVC
 *
 * Bulk transfers for UVC need multiple interleaved transfers to capture
 * complete frames.
 */
class UVCBulkTransfer : public Transfer {
	size_t	payloadtransfersize;
	size_t	maxframesize;	
	int	nframes;
	int	ntransfers;
	int	queuesize;
	int	submitted;
	libusb_transfer	**transfers;
	unsigned char	**buffers;
private:
        virtual void    submit(libusb_device_handle *devhandle) throw(USBError);
public:
	std::vector<FramePtr>	frames;
	std::list<std::string>	packets;
	UVCBulkTransfer(EndpointDescriptorPtr endpoint, int nframes,
		size_t payloadtransfersize, size_t framesize);
	virtual	~UVCBulkTransfer();
	virtual void	callback(libusb_transfer *transfer);
};

/**
 * \brief Iso transfer for UVC
 *
 * Isochronous transfer for UVC also uses interleaved transfers
 */
class UVCIsochronousTransfer : public Transfer {
	int	nframes;	// number of frames to read
	int	ntransfers;	// number of transfers in transfers array
	int	queuesize;	// number of transfers in queue
	int	submitted;	// number of transfers submitted
	int	completed;	// number of transfers that actually did 
				// bring in some data
	int	frameinterval;
	int	packetsize;
	unsigned long	bytestransferred;
	libusb_transfer	**transfers;
	unsigned char	**buffers;
private:
        virtual void    submit(libusb_device_handle *devhandle) throw(USBError);
public:
	std::vector<FramePtr>	frames;
	std::list<std::string>	packets;
	UVCIsochronousTransfer(EndpointDescriptorPtr endpoint, int nframes,
		int frameinterval);
	virtual	~UVCIsochronousTransfer();
	virtual void	callback(libusb_transfer *transfer);
};

/**
 * \brief Frame factory
 *
 * The transfer functions return a queue 
 */
class FrameFactory {
	int	width;
	int	height;
	int	bytesperpixel;
public:
	FrameFactory(int width, int height, int bytesperpixel);
	std::vector<FramePtr>	operator()(const std::list<std::string>& packets) const;
};

} // namespace uvc
} // namespace usb
} // namespace astro

#endif /* _AstroUVC_h */
