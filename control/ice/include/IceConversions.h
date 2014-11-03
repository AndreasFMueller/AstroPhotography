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
#include <types.h>
#include <device.h>
#include <camera.h>
#include <guider.h>
#include <focusing.h>
#include <tasks.h>
#include <repository.h>
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

ShutterState	convert(const astro::camera::shutter_state& state);
astro::camera::shutter_state	convert(const ShutterState& state);

ExposurePurpose	convert(const astro::camera::Exposure::purpose_t& state);
astro::camera::Exposure::purpose_t	convert(const ExposurePurpose& state);

// FilterWheel
FilterwheelState convert(const astro::camera::FilterWheel::State& s);
astro::camera::FilterWheel::State convert(const FilterwheelState& s);

// tracking related

// Guider related
GuiderState     convert(const astro::guiding::GuiderState& state);
astro::guiding::GuiderState     convert(const GuiderState& state);

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

TaskInfo	convert(const astro::task::TaskInfo& info);
astro::task::TaskInfo	convert(const TaskInfo& info);

TaskParameters	convert(const astro::task::TaskParameters& parameters);
astro::task::TaskParameters	convert(const TaskParameters& parameters);

QueueState      convert(const astro::task::TaskQueue::state_type& state);
astro::task::TaskQueue::state_type	convert(const QueueState& state);

TaskMonitorInfo	convert(const astro::task::TaskMonitorInfo& monitorinfo);
astro::task::TaskMonitorInfo	convert(const TaskMonitorInfo& monitorinfo);

// Mount
mountstate	convert(astro::device::Mount::mount_state s);
astro::device::Mount::mount_state	convert(mountstate s);

// Image
astro::image::ImagePtr	convert(ImagePrx image);

astro::image::ImagePtr	convertsimple(SimpleImage image);
SimpleImage	convertsimple(astro::image::ImagePtr image);

astro::image::ImagePtr	convertfile(ImageFile imagefile);
ImageFile	convertfile(astro::image::ImagePtr imageptr);

// Focusing
FocusState	convert(astro::focusing::Focusing::focus_status s);
astro::focusing::Focusing::focus_status	convert(FocusState s);

FocusMethod	convert(astro::focusing::Focusing::focus_method m);
astro::focusing::Focusing::focus_method	convert(FocusMethod m);

// Repository related conversions
ImageInfo	convert(const astro::project::ImageEnvelope& envelope);
astro::project::ImageEnvelope	convert(const ImageInfo& info);

} // namespace snowstar

#endif /* _IceConversions_h */
