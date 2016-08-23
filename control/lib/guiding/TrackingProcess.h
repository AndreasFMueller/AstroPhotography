/*
 * TrackingProcess.h -- definition of the tracking process
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <BasicProcess.h>

namespace astro {
namespace guiding {

/**
 * \brief Tracking class
 */
class TrackingProcess : public BasicProcess {
	ControlDevicePtr	_guiderPortDevice;
	ControlDevicePtr	_adaptiveOpticsDevice;
	TrackerPtr	_tracker;
	int	_id;
	double	_gain;
public:
	double	gain() const { return _gain; }
	void	gain(double g) { _gain = g; }
private:
	double	_guiderportInterval;
public:
	double	guiderportInterval() const {
		return _guiderportInterval;
	}
	void	guiderportInterval(double g) {
		_guiderportInterval = g;
	}
private:
	double	_adaptiveopticsInterval;
public:
	double	adaptiveopticsInterval() const {
		return _adaptiveopticsInterval;
	}
	void	adaptiveopticsInterval(double a) {
		_adaptiveopticsInterval = a;
	}

	bool	adaptiveOpticsUsable();
	bool	guiderPortUsable();

private:
	bool	_stepping;
public:
	bool	stepping() const { return _stepping; }
	void	stepping(bool s) { _stepping = s; }

private:
	callback::CallbackPtr	_callback;
	TrackingPoint	_last;
public:
	void	callback(const TrackingPoint& trackingpoint);
	const TrackingPoint&	last() const { return _last; }

	// constructor
	TrackingProcess(GuiderBase *base, TrackerPtr tracker,
		ControlDevicePtr guiderPortDevice,
		ControlDevicePtr adaptiveOpticsDevice,
		persistence::Database database);
	~TrackingProcess();

	void    main(astro::thread::Thread<TrackingProcess>& thread);
private:
	void	step(astro::thread::Thread<TrackingProcess>& thread,
			double imageiInterval, double& guiderportTime);

private:
	TrackingSummary	_summary;
public:
	const TrackingSummary&	summary() const { return _summary; }
};

} // namespace guiding
} // namespace astro
