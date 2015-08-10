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
#include <TrackingPersistence.h>
#include <AstroDiscovery.h>
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

Interval	convert(const std::pair<float, float>& interval);
std::pair<float, float>	convert(const Interval& interval);

// Parameters
struct ParameterDescription	convert(const astro::device::ParameterDescription& parameter);
astro::device::ParameterDescription	convert(const struct ParameterDescription& parameter);

// Device conversions
DeviceNameList  convert(const astro::module::Devices::devicelist& list);
astro::module::Devices::devicelist	convert(const DeviceNameList& list);

devicetype       convert(const astro::DeviceName::device_type& type);
astro::DeviceName::device_type   convert(const devicetype& type);

// CCD related
BinningMode	convert(const astro::camera::Binning& binning);
astro::camera::Binning	convert(const BinningMode& mode);

BinningSet	convert(const astro::camera::BinningSet& binningset);
astro::camera::BinningSet	convet(const BinningSet& binningset);

CcdInfo convert(const astro::camera::CcdInfo& info);
astro::camera::CcdInfo convert(const CcdInfo& info);

Exposure	convert(const astro::camera::Exposure& exp);
astro::camera::Exposure convert(const Exposure& exp);

ExposureState	convert(const astro::camera::Exposure::State& state);
astro::camera::Exposure::State  convert(const ExposureState& state);

ShutterState	convert(const astro::camera::Shutter::state& state);
astro::camera::Shutter::state	convert(const ShutterState& state);

std::string	state2string(ShutterState s);
ShutterState	string2shutterstate(const std::string& s);

ExposurePurpose	convert(const astro::camera::Exposure::purpose_t& state);
astro::camera::Exposure::purpose_t	convert(const ExposurePurpose& state);

// FilterWheel
FilterwheelState convert(const astro::camera::FilterWheel::State& s);
astro::camera::FilterWheel::State convert(const FilterwheelState& s);

// tracking related

// Guider related
GuiderState     convert(const astro::guiding::Guide::state& state);
astro::guiding::Guide::state     convert(const GuiderState& state);

std::string	guiderstate2string(GuiderState state);
GuiderState	string2guiderstate(const std::string& s);

GuiderDescriptor	convert(const astro::guiding::GuiderDescriptor& gd);
astro::guiding::GuiderDescriptor	convert(const GuiderDescriptor& gd);

TrackingPoint	convert(const astro::guiding::TrackingPoint& trackingpoint);
astro::guiding::TrackingPoint	convert(const TrackingPoint& trackingpoint);

TrackingHistory	convert(const astro::guiding::TrackingHistory& history);
astro::guiding::TrackingHistory	convert(const TrackingHistory& history);

CalibrationPoint	convert(const astro::guiding::CalibrationPoint& cp);
astro::guiding::CalibrationPoint	convert(const CalibrationPoint& cp);

// TaskQueue
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

// Mount
mountstate	convert(astro::device::Mount::state_type s);
astro::device::Mount::state_type	convert(mountstate s);

std::string	state2string(mountstate s);
mountstate	string2mountstate(const std::string& s);

// Image
astro::image::ImagePtr	convert(ImagePrx image);

astro::image::ImagePtr	convertsimple(SimpleImage image);
SimpleImage	convertsimple(astro::image::ImagePtr image);

astro::image::ImagePtr	convertfile(ImageFile imagefile);
ImageFile	convertfile(astro::image::ImagePtr imageptr);

// Focusing
FocusState	convert(astro::focusing::Focusing::state_type s);
astro::focusing::Focusing::state_type	convert(FocusState s);

std::string	focusingstate2string(FocusState s);
FocusState	focusingstring2state(const std::string& s);

FocusMethod	convert(astro::focusing::Focusing::method_type m);
astro::focusing::Focusing::method_type	convert(FocusMethod m);

std::string	focusingmethod2string(FocusMethod m);
FocusMethod	focusingstring2method(const std::string& m);

// Repository related conversions
ImageInfo	convert(const astro::project::ImageEnvelope& envelope);
astro::project::ImageEnvelope	convert(const ImageInfo& info);

// Instrument related conversions
InstrumentComponent	convert(const astro::discover::InstrumentComponent& component);
astro::discover::InstrumentComponent	convert(const struct InstrumentComponent& component);

InstrumentComponentList	convert(const astro::discover::Instrument::ComponentList& list);
astro::discover::Instrument::ComponentList	convert(const InstrumentComponentList& list);

InstrumentList	convert(const astro::discover::InstrumentList &list);
astro::discover::InstrumentList	convertInstrumentList(const InstrumentList& list);

} // namespace snowstar

#endif /* _IceConversions_h */
