/*
 * monitor.h -- monitoring callback classes
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _monitor_h
#define _monitor_h

#include <mutex>
#include <guider.h>
#include "display.h"
#include <ImageCallbackI.h>

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief Common infrastructure for monitor classes
 */
class CommonMonitor {
	std::mutex	mtx;
	std::condition_variable	cond;
	bool	_complete;
public:
	bool	complete() const { return _complete; }
	void	complete(bool c);
	CommonMonitor();
	void	wait();
};

/**
 * \brief Calibration monitor class
 */
class CalibrationMonitorI : public CalibrationMonitor, public CommonMonitor {
	CalibrationPoint_display	display;
public:
	CalibrationMonitorI();
	void	update(const CalibrationPoint& point,
		const Ice::Current& current);
	void	stop(const Ice::Current& current);
};

/**
 * \brief Tracking monitor class
 */
class TrackingMonitorI : public TrackingMonitor, public CommonMonitor {
	TrackingPoint_display	display;
public:
	bool	csv() const;
	void	csv(bool c);
	TrackingMonitorI();
	void	update(const TrackingPoint& point, const Ice::Current& current);
	void	stop(const Ice::Current& current);
};

/**
 * \brief 
 */
class GuideImageMonitor : public ImageCallbackI, public CommonMonitor {
public:
	GuideImageMonitor(const std::string& path, const std::string& prefix)
		: ImageCallbackI(path, prefix) { }
};

/*
 * \brief monitor tracking
 */
extern int	monitor_guiding(GuiderPrx guider);

/*
 * \brief monitor calibration
 */
extern int	monitor_calibration(GuiderPrx guider);

} // namespace snowguide
} // namespace app
} // namespace snowstar

#endif /* _monitor_h */
