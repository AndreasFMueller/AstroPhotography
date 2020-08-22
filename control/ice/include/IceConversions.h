/*
 * IceConversions.h -- conversions from Ice types to astro types and back
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _IceConversions_h
#define _IceConversions_h

#include <AstroCamera.h>
#include <AstroGuiding.h>
#include <AstroTask.h>
#include <AstroDevice.h>
#include <AstroFocus.h>
#include <AstroProject.h>
#include <AstroEvent.h>
#include <AstroConfig.h>
#include <AstroDiscovery.h>
#include <AstroGateway.h>
#include <types.h>
#include <device.h>
#include <camera.h>
#include <guider.h>
#include <focusing.h>
#include <tasks.h>
#include <repository.h>
#include <instruments.h>
#include <sys/time.h>

namespace snowstar {

// time conversions
time_t	converttime(double timeago);
double	converttime(time_t t);

struct timeval	converttimeval(double timeago);
double	converttimeval(struct timeval t);

// Types
ImagePoint	convert(const astro::image::ImagePoint& point);
astro::image::ImagePoint	convert(const ImagePoint& point);

ImageSize	convert(const astro::image::ImageSize& size);
astro::image::ImageSize	convert(const ImageSize& size);

ImageRectangle	convert(const astro::image::ImageRectangle& rectangle);
astro::image::ImageRectangle	convert(const ImageRectangle& rectangle);

Point	convert(const astro::Point& point);
astro::Point	convert(const Point& point);

RaDec	convert(const astro::RaDec& radec);
astro::RaDec	convert(const RaDec& radec);

AzmAlt	convert(const astro::AzmAlt& azmalt);
astro::AzmAlt	convert(const AzmAlt& azmalt);

LongLat	convert(const astro::LongLat& longlat);
astro::LongLat	convert(const LongLat& longlat);

Interval	convert(const std::pair<float, float>& interval);
std::pair<float, float>	convert(const Interval& interval);

// Parameters
struct ParameterDescription	convert(
	const astro::device::ParameterDescription& parameter);
astro::device::ParameterDescription	convert(
	const struct ParameterDescription& parameter);

struct ConfigurationKey	convert(const astro::config::ConfigurationKey& key);
astro::config::ConfigurationKey	convert(const struct ConfigurationKey& key);

struct ConfigurationItem	convert(const astro::config::ConfigurationEntry& entry);
astro::config::ConfigurationEntry	convert(const struct ConfigurationItem& entry);

// Device conversions
DeviceNameList  convert(const astro::module::Devices::devicelist& list);
astro::module::Devices::devicelist	convert(const DeviceNameList& list);

devicetype       convert(const astro::DeviceName::device_type& type);
astro::DeviceName::device_type   convert(const devicetype& type);

// CCD related
BinningMode	convert(const astro::image::Binning& binning);
astro::image::Binning	convert(const BinningMode& mode);

BinningSet	convert(const astro::camera::BinningSet& binningset);
astro::camera::BinningSet	convert(const BinningSet& binningset);

CcdInfo convert(const astro::camera::CcdInfo& info);
astro::camera::CcdInfo convert(const CcdInfo& info);

Exposure	convert(const astro::camera::Exposure& exp);
astro::camera::Exposure convert(const Exposure& exp);

ExposureState	convert(const astro::camera::CcdState::State& state);
astro::camera::CcdState::State  convert(const ExposureState& state);

ShutterState	convert(const astro::camera::Shutter::state& state);
astro::camera::Shutter::state	convert(const ShutterState& state);
std::string	state2string(ShutterState s);
ShutterState	string2shutterstate(const std::string& s);

ExposurePurpose	convert(const astro::camera::Exposure::purpose_t& state);
astro::camera::Exposure::purpose_t	convert(const ExposurePurpose& state);

astro::camera::Exposure::quality_t	convert(const ExposureQuality quality);
ExposureQuality	convert(const astro::camera::Exposure::quality_t& quality);
std::string	quality2string(const ExposureQuality& quality);
ExposureQuality	string2quality(const std::string& s);

typedef std::shared_ptr<ImageQueueEntry>	ImageQueueEntryPtr;

ImageQueueEntryPtr	convert(const astro::camera::ImageQueueEntry e);
astro::camera::ImageQueueEntry	convert(ImageQueueEntryPtr e);

// Cooler
CoolerInfo	convert(const astro::camera::CoolerInfo& ci);
astro::camera::CoolerInfo	convert(const CoolerInfo& ci);

// FilterWheel
FilterwheelState convert(const astro::camera::FilterWheel::State& s);
astro::camera::FilterWheel::State convert(const FilterwheelState& s);

// Guider related
GuidePortActivation	convert(const astro::camera::GuidePortActivation& a);
astro::camera::GuidePortActivation	convert(const GuidePortActivation& a);

GuiderState     convert(const astro::guiding::Guide::state& state);
astro::guiding::Guide::state     convert(const GuiderState& state);

std::string	guiderstate2string(GuiderState state);
GuiderState	string2guiderstate(const std::string& s);

GuiderDescriptor	convert(const astro::guiding::GuiderDescriptor& gd);
astro::guiding::GuiderDescriptor	convert(const GuiderDescriptor& gd);

GuiderDescriptor	convertname(const astro::guiding::GuiderName& name);
astro::guiding::GuiderName	convertname(const GuiderDescriptor& name);

ControlType	convertcontroltype(const astro::guiding::ControlDeviceType& caltype);
astro::guiding::ControlDeviceType	convertcontroltype(const ControlType& caltype);

std::string	calibrationtype2string(ControlType caltype);
ControlType	string2calibrationtype(const std::string& caltype);

// tracking related
TrackingPoint	convert(const astro::guiding::TrackingPoint& trackingpoint);
astro::guiding::TrackingPoint	convert(const TrackingPoint& trackingpoint);

TrackingHistory	convert(const astro::guiding::TrackingHistory& history);
astro::guiding::TrackingHistory	convert(const TrackingHistory& history);

TrackingSummary	convert(const astro::guiding::TrackingSummary& summary);
astro::guiding::TrackingSummary	convert(const TrackingSummary& summary);

// calibration related
CalibrationPoint	convert(const astro::guiding::CalibrationPoint& cp);
astro::guiding::CalibrationPoint	convert(const CalibrationPoint& cp);

Calibration	convert(const astro::guiding::CalibrationPtr cal);
astro::guiding::CalibrationPtr	convert(const Calibration& cal);

std::string	guiderdescriptor2name(const GuiderDescriptor& descriptor);
GuiderDescriptor	guiderdescriptorParse(const std::string &name);

// Calibration image related
CalibrationImageProgress	convert(const astro::camera::CalibrationImageProgress);
astro::camera::CalibrationImageProgress	convert(const CalibrationImageProgress);

// Backlash related
BacklashPoint	convert(const astro::guiding::BacklashPoint& point);
astro::guiding::BacklashPoint	convert(const BacklashPoint& point);

BacklashPoints	convert(const astro::guiding::BacklashPoints& points);
astro::guiding::BacklashPoints	convert(const BacklashPoints& points);

BacklashResult	convert(const astro::guiding::BacklashResult& result);
astro::guiding::BacklashResult	convert(const BacklashResult& result);

BacklashData	convert(const astro::guiding::BacklashData& data);
astro::guiding::BacklashData	convert(const BacklashData& data);

BacklashDirection	convert(const astro::guiding::backlash_t);
astro::guiding::backlash_t	convert(BacklashDirection);

// TaskQueue
TaskType	convert(const astro::task::tasktype& type);
astro::task::tasktype	convert(const TaskType& type);

std::string	tasktype2string(const TaskType& type);
TaskType	string2tasktype(const std::string& type);

TaskState	convert(const astro::task::TaskInfo::taskstate& state);
astro::task::TaskInfo::taskstate	convert(const TaskState& state);

std::string	state2string(TaskState s);
TaskState	string2taskstate(const std::string& s);

TaskInfo	convert(const astro::task::TaskInfo& info);
astro::task::TaskInfo	convert(const TaskInfo& info);

TaskParameters	convert(const astro::task::TaskParameters& parameters);
astro::task::TaskParameters	convert(const TaskParameters& parameters);

QueueState      convert(const astro::task::TaskQueue::state_type& state);
astro::task::TaskQueue::state_type	convert(const QueueState& state);

TaskMonitorInfo	convert(const astro::task::TaskMonitorInfo& monitorinfo);
astro::task::TaskMonitorInfo	convert(const TaskMonitorInfo& monitorinfo);

std::string	state2string(QueueState s);

astro::gateway::TaskUpdate	convert(const StatusUpdate& statusupdate);
StatusUpdate	convert(const astro::gateway::TaskUpdate& taskupdate);

// Mount
mountstate	convert(astro::device::Mount::state_type s);
astro::device::Mount::state_type	convert(mountstate s);

std::string	state2string(mountstate s);
mountstate	string2mountstate(const std::string& s);

// Image
astro::image::ImagePtr	convert(ImagePrx image);

astro::image::ImagePtr	convertfile(ImageFile imagefile);
ImageFile	convertfile(astro::image::ImagePtr imageptr);

astro::image::Metavalue	convert(const Metavalue& metavalue);
Metavalue	convert(const astro::image::Metavalue& metavalue);

astro::image::Format::type_t	convert(ImageEncoding e);
ImageEncoding	convert(astro::image::Format::type_t t);

astro::image::ImageBufferPtr	convert(const ImageBuffer& imagebuffer);
typedef std::shared_ptr<ImageBuffer>	ImageBufferPtr;
ImageBufferPtr	convert(const astro::image::ImageBuffer& imagebuffer);

astro::image::ImagePtr	convertimage(const ImageBuffer& imagebuffer);

// Focusing
FocusState	convert(astro::focusing::Focus::state_type s);
astro::focusing::Focus::state_type	convert(FocusState s);

std::string	focusingstate2string(FocusState s);
FocusState	focusingstring2state(const std::string& s);

FocusPoint	convert(const astro::focusing::FocusItem& fi);
astro::focusing::FocusItem	convert(const FocusPoint& fp);

typedef std::shared_ptr<FocusElement>	FocusElementPtr;
FocusElementPtr	convert(const astro::focusing::FocusElement& fe,
			astro::image::Format::type_t type);
FocusElementPtr	convert(const astro::focusing::FocusElementCallbackData& fe,
			astro::image::Format::type_t type);
astro::focusing::FocusElementPtr	convert(const FocusElement& fe);


// Repository related conversions
ImageInfo	convert(const astro::project::ImageEnvelope& envelope);
astro::project::ImageEnvelope	convert(const ImageInfo& info);

// Instrument related conversions
InstrumentComponent	convert(
	const astro::discover::InstrumentComponent& component);
astro::discover::InstrumentComponent	convert(
	const struct InstrumentComponent& component);

InstrumentComponentList	convert(
	const astro::discover::Instrument::ComponentList& list);
astro::discover::Instrument::ComponentList	convert(
	const InstrumentComponentList& list);

InstrumentList	convert(const astro::discover::InstrumentList &list);
astro::discover::InstrumentList	convertInstrumentList(
	const InstrumentList& list);

InstrumentComponentType	name2instrumentcomponent(const std::string& name);
std::string	instrumentcomponent2name(const InstrumentComponentType type);

InstrumentComponentType	convertInstrumentType(
	astro::discover::InstrumentComponentKey::Type type);
astro::discover::InstrumentComponentKey::Type	convertInstrumentType(
	InstrumentComponentType type);

int	instrumentName2index(const std::string& instrument,
		const InstrumentComponentType type,
		const std::string& deviceurl);
std::string	instrumentIndex2name(const std::string& instrument,
		const InstrumentComponentType type, int index);

astro::discover::InstrumentProperty	convert(const InstrumentProperty& p);
InstrumentProperty	convert(const astro::discover::InstrumentProperty& p);

astro::discover::Instrument::PropertyNames	convertPropertyNames(const InstrumentPropertyNames& names);
InstrumentPropertyNames	convertPropertyNames(const astro::discover::Instrument::PropertyNames& names);

astro::discover::InstrumentPropertyList	convert(const InstrumentPropertyList& properties);
InstrumentPropertyList	convert(const astro::discover::InstrumentPropertyList& properties);

// Events
snowstar::EventLevel	convert(astro::events::eventlevel_t);
astro::events::eventlevel_t	convert(snowstar::EventLevel);

snowstar::Event	convert(const astro::events::Event& e);
astro::events::Event	convert(const snowstar::Event& e);

snowstar::Event	convert(const astro::events::EventRecord& e);
astro::events::EventRecord	convertRecord(const snowstar::Event& e);

} // namespace snowstar

#endif /* _IceConversions_h */
