/*
 * Conversions.h -- conversion functions between different classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#ifndef _Conversions_h
#define _Conversions_h

#include <module.hh>
#include <AstroTypes.h>
#include <AstroLocator.h>
#include <AstroCamera.h>
#include <guider.hh>
#include <camera.hh>
#include <AstroGuiding.h>
#include <AstroTask.h>
#include <tasks.hh>
#include <TrackingPersistence.h>
#include <CalibrationPersistence.h>

namespace astro {

// Device type
astro::DeviceName::device_type convert(
			Astro::DeviceLocator::device_type fromtype);
Astro::DeviceLocator::device_type convert(
			astro::DeviceName::device_type fromtype);
std::string	convert2string(Astro::DeviceLocator::device_type fromtype);
std::string	convert2string(
			astro::DeviceName::device_type fromtype);

// Exposure state
astro::camera::Exposure::State	convert(Astro::ExposureState fromstate);
Astro::ExposureState	convert(astro::camera::Exposure::State fromstate);
std::string	convert2string(Astro::ExposureState fromstate);
std::string	convert2string(astro::camera::Exposure::State fromstate);

// ImagePoint
astro::image::ImagePoint	convert(const Astro::ImagePoint& point);
Astro::ImagePoint	convert(const astro::image::ImagePoint& point);

// Point
astro::Point	convert(const Astro::Point& point);
Astro::Point	convert(const astro::Point& point);

// ImageSize
astro::image::ImageSize	convert(const Astro::ImageSize& size);
Astro::ImageSize	convert(const astro::image::ImageSize& size);

// ImageRectangle
astro::image::ImageRectangle	convert(const Astro::ImageRectangle& rectangle);
Astro::ImageRectangle	convert(const astro::image::ImageRectangle& rectangle);

// shutter state
astro::camera::shutter_state	convert(const Astro::ShutterState state);
Astro::ShutterState	convert(const astro::camera::shutter_state state);

// exposure purpose
astro::camera::Exposure::purpose_t	convert(const Astro::ExposurePurpose purpose);
Astro::ExposurePurpose convert(const astro::camera::Exposure::purpose_t purpose);

// Binning mode
astro::camera::Binning	convert(const Astro::BinningMode& mode);
Astro::BinningMode	convert(const astro::camera::Binning& mode);

// BinningSet
astro::camera::BinningSet	convert(const Astro::BinningSet_var set);
astro::camera::BinningSet	convert(const Astro::BinningSet set);
Astro::BinningSet_var	convert(const astro::camera::BinningSet& set);

// Exposure
astro::camera::Exposure	convert(const Astro::Exposure& exposure);
Astro::Exposure	convert(const astro::camera::Exposure& exposure);

// relay bits
uint8_t	convert_octet2relaybits(CORBA::Octet bits);
CORBA::Octet	convert_relaybits2octet(uint8_t bits);

// CcdInfo
astro::camera::CcdInfo	convert(const Astro::CcdInfo& info);
astro::camera::CcdInfo	convert(const Astro::CcdInfo_var& info);
Astro::CcdInfo	*convert(const astro::camera::CcdInfo& info);

// GuiderDescriptor
astro::guiding::GuiderDescriptor        convert(
        const Astro::GuiderDescriptor& guiderdescriptor);
Astro::GuiderDescriptor  convert(
        const astro::guiding::GuiderDescriptor& guiderdescriptor);

// FilterWheel state
astro::camera::FilterWheel::State	convert(const Astro::FilterwheelState& state);
Astro::FilterwheelState	convert(const astro::camera::FilterWheel::State& state);

// GuiderCalibration
astro::guiding::GuiderCalibration	convert(const Astro::Calibration& cal);

// TaskState
astro::task::TaskQueueEntry::taskstate	convert(const Astro::TaskState& state);
Astro::TaskState	convert(const astro::task::TaskQueueEntry::taskstate& state);

// TaskQueueState
astro::task::TaskQueue::state_type      convert(const Astro::TaskQueue::QueueState state);
Astro::TaskQueue::QueueState	convert(const astro::task::TaskQueue::state_type     state);

// TaskParameters
astro::task::TaskParameters	convert(const Astro::TaskParameters& parameters);
Astro::TaskParameters	convert(const astro::task::TaskParameters& task);

// TaskInfo
astro::task::TaskInfo	convert(const Astro::TaskInfo& info);
Astro::TaskInfo	convert(const astro::task::TaskInfo& info);

// GuiderState
astro::guiding::GuiderState	convert(const Astro::Guider::GuiderState& state);
Astro::Guider::GuiderState	convert(const astro::guiding::GuiderState& state);

// TrackingPoint
astro::guiding::TrackingPoint	convert(const Astro::TrackingPoint& trackinginfo);
Astro::TrackingPoint	convert(const astro::guiding::TrackingPoint& trackinginfo);

// CalibrationPoint
astro::guiding::CalibrationPoint	convert(const Astro::CalibrationPoint& calibrationpoint);
Astro::CalibrationPoint	convert(const astro::guiding::CalibrationPoint& calibrationpoint);

// TaskMonitorInfo
astro::task::TaskMonitorInfo	convert(const Astro::TaskMonitorInfo& tmi);
Astro::TaskMonitorInfo	convert(const astro::task::TaskMonitorInfo& tmi);

} // namespace astro

#endif /* _Conversions_h */
