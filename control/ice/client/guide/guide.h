/*
 * guide.h -- main class for the guide client program
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _guide_h
#define _guide_h

#include <string>
#include <guider.h>
#include <list>
#include "monitor.h"

namespace snowstar {
namespace app {
namespace snowguide {

class Guide {
public:
	bool	verbose;
	snowstar::ImagePoint	star;
	Exposure	exposure;
	std::string	prefix;
	volatile bool	completed;
	double	guideinterval;
	double	aointerval;
	bool	csv;
	bool	flipped;
	bool	stepping;
	TrackerMethod	method;
private:
	CommonMonitor	*monitor;
public:

	Guide() {
		verbose = false;
		prefix = std::string("p");
		completed = false;
		guideinterval = 10;
		aointerval = 0;
		csv = false;
		monitor = NULL;
		method = TrackerUNDEFINED;
		flipped = false;
		stepping = false;
	}

	// general commands
	void	usage(const char *progname);
	int	help_command(const char *progname);
	int	state_command(GuiderPrx guider);
	int	repository_command(GuiderPrx guider);
	int	repository_command(GuiderPrx guider,
			const std::string& repository);

	// monitoring
public:
	void	complete(bool);
	int	monitor_calibration(GuiderPrx guider);
	int	monitor_guiding(GuiderPrx guider);
public:
	int	monitor_command(GuiderPrx guider);
	int	images_command(GuiderPrx guider, const std::string& path);

	// commands related to calibration
	int	calibration_command(GuiderFactoryPrx guiderfactory,
			GuiderPrx guider);
	int	calibration_command(GuiderFactoryPrx guiderfactory,
			int calibrationid);
	int	calibration_command(GuiderFactoryPrx guiderfactory,
			GuiderPrx guider, const std::string& calarg);
	void	calibration_show(const Calibration& cal);

	int	calibrate_command(GuiderPrx guider);
	int	calibrate_command(GuiderPrx guider, int calibrationid);
	int	calibrate_command(GuiderPrx guider, const std::string& calarg);
	int	uncalibrate_command(GuiderPrx guider, ControlType type);
	int	flip_command(GuiderPrx guider);
	int	flip_command(GuiderPrx guider, ControlType type);

	int	cancel_command(GuiderPrx guider);
	int	list_command(GuiderFactoryPrx guiderfactory,
			GuiderDescriptor descriptor);
	int	trash_command(GuiderFactoryPrx guiderfactory,
			std::list<int> ids);

	// commands related to guiding/tracking
	int	guide_command(GuiderPrx guider);
	int	stop_command(GuiderPrx guider);
	int	tracks_command(GuiderFactoryPrx guiderfactory,
			GuiderDescriptor descriptor);
	int	history_command(GuiderFactoryPrx guiderfactory, long historyid);
	int	history_command(GuiderFactoryPrx guiderfactory, long historyid,
			ControlType type);
	int	forget_command(GuiderFactoryPrx guiderfactory,
			const std::list<int>& ids);

	// utility functions
static	ControlType	string2type(const std::string& type);
};

} // namespace snowguide
} // namespace app
} // namespace snowstar

#endif /* _guide_h */
