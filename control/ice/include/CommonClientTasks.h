/*
 * CommonClientTasks.h -- common stuff used in multiple ICE clients
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CommonClientTasks_h
#define _CommonClientTasks_h

#include <AstroFormat.h>
#include <camera.h>
#include <AstroCamera.h>
#include <Ice/Ice.h>
#include <mutex>
#include <condition_variable>
#include <RemoteInstrument.h>

namespace snowstar {

/**
 * \brief tasks related to the ccd
 *
 * This task allows to set up the ccd exposure
 */
class CcdTask {
	CcdPrx&	_ccd;
	astro::camera::Exposure	_exposure;
public:
	CcdTask(CcdPrx& ccd);
	void	frame(const astro::image::ImageRectangle& frame);
	void	frame(const std::string& framespec);
	void	binning(const astro::image::Binning& binning);
	void	binning(const std::string& binning);
	void	exposuretime(double exposuretime);
	void	purpose(const astro::camera::Exposure::purpose_t purpose);
	void	purpose(const std::string& purposename);
	void	shutter(astro::camera::Shutter::state shutter);
	Exposure	exposure() const;
	void	start();
	void	wait(int timeout = 60);
	void	available(int timeout = 60);
};
typedef std::shared_ptr<CcdTask>	CcdTaskPtr;

/**
 * \brief tasks related to the cooler
 *
 * This task sets up the cooler and waits for the temperature to be reached
 * By setting the stop_on_exit flag to true, one can ensure that the 
 * cooler is turned off when the task goes out of scope. Default is
 * not to turn off the cooler.
 */
class CoolerTask {
	CoolerPrx	_cooler;
	double	_absolute;
	bool	we_turned_cooler_on;
	bool	_stop_on_exit;
	void	setup(double temperature);
public:
	bool	stop_on_exit() const { return _stop_on_exit; }
	void	stop_on_exit(bool s) { _stop_on_exit = s; }
	
	CoolerTask(CoolerPrx cooler, double temperature);
	CoolerTask(RemoteInstrument& ri, double temperature);
	void	wait(int timeout = 300);
	void	stop();
	~CoolerTask();
};
typedef std::shared_ptr<CoolerTask>	CoolerTaskPtr;

/**
 * \brief Task realted to the focuser
 *
 * The constructor of this task moves the focuser to a given position and
 * waits for the movement to complete
 */
class FocuserTask {
	FocuserPrx	_focuser;
	int	_position;
	bool	we_started_focuser;
	void	setup();
public:
	FocuserTask(FocuserPrx focuser, int position);
	FocuserTask(RemoteInstrument& ri, int position);
	void	wait(int timeout = 60);
};
typedef std::shared_ptr<FocuserTask>	FocuserTaskPtr;

/**
 * \brief Task related to the filterwheel
 *
 * The constructor of this task moves 
 */
class FilterwheelTask {
	FilterWheelPrx	_filterwheel;
	const std::string	_filtername;
	bool	we_started_filterwheel;
	void	setup();
public:
	FilterwheelTask(FilterWheelPrx filterwheel,
		const std::string& filtername);
	FilterwheelTask(RemoteInstrument& ri, const std::string& filtername);
	void	wait(int timeout = 60);
};
typedef std::shared_ptr<FilterwheelTask>	FilterwheelTaskPtr;

/**
 * \brief Callback adapter
 *
 * Note: this is not a good architecture for the client side of the callbacks.
 * Use CommunicatorSingleton instead
 */
class CallbackAdapter {
	Ice::ObjectAdapterPtr	_adapter;
public:
	Ice::ObjectAdapterPtr	adapter() { return _adapter; }
	CallbackAdapter(Ice::CommunicatorPtr communicator);
	Ice::Identity	add(Ice::ObjectPtr callback);
	void	connect(IceProxy::Ice::Object& proxy);
};
typedef std::shared_ptr<CallbackAdapter>	CallbackAdapterPtr;

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

} // namespace snowstar

#endif /* _CommonClientTasks_h */
