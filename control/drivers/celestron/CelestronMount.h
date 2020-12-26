/*
 * CelestronMount.h -- interface class for Celestron mounts
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _CelestronMount_h
#define _CelestronMount_h

#include <AstroDevice.h>
#include <Serial.h>
#include <mutex>
#include <thread>
#include <atomic>

namespace astro {
namespace device {
namespace celestron {

class CelestronMount : public astro::device::Mount,
			public astro::device::Serial {
	// mutex to protect serial communication from concurrent commands
	std::recursive_mutex		_mount_mutex;
	// the thread monitors the mount 
	std::thread			_mount_thread;
	std::condition_variable_any	_mount_condition;
	std::atomic_bool		_running;
	void	stop_thread();
	void	start_thread();
	void	check_state();
	void	run();
	static void	launch(CelestronMount *mount) noexcept;
private:
	// Goto launches a thread, no other communication with the mount
	// is allowed until the Goto is completed, and the communication
	// is entirely handled by the thread

	// basic information for communication
	int	version;
	void	getprompt();

	// auxiliary functions for angles
	double	angle(uint16_t a);
	double	angle(uint32_t a);
	uint16_t	angle16(const Angle& a);
	uint32_t	angle32(const Angle& a);
	std::pair<double, double>	parseangles(const std::string& s);

	// commands related to the 
	std::vector<uint8_t>	gps_command(uint8_t a, uint8_t, size_t l);
	bool	gps_linked();
	Angle	gps_longitude();
	Angle	gps_latitude();
	typedef struct { int month; int day; } gps_date_t;
	gps_date_t	gps_date();
	int	gps_year();
	typedef struct { int hour; int minute; int seconds; } gps_time_t;
	gps_time_t	gps_time();

	static const int	query_interval;

	// the time() method could be called very often which might 
	// interfere with the telescope operation. 
	int	_last_time_offset;
	time_t	_last_time_queried;

	// also we prevent the location from being required all the time
	time_t	_last_location_queried;
	location_source_type	_last_location_source;

	bool	queriable(time_t last);
	astro::device::Mount::state_type	get_state();

public:
	CelestronMount(const std::string& devicename);
	virtual ~CelestronMount();

	// accessors
	virtual RaDec	getRaDec();
	virtual AzmAlt	getAzmAlt();
	virtual LongLat	location();
	virtual location_source_type	location_source();
	virtual time_t	time();
	virtual void	Goto(const RaDec& radec);
	virtual void	Goto(const AzmAlt& azmalt);
	virtual bool	telescopePositionWest();
	virtual void	cancel();

	virtual bool	hasGuideRates();
	virtual RaDec	getGuideRates();
};

} // namepace celestron
} // namespace device
} // namespace astro

#endif /* _CelestronMount */
