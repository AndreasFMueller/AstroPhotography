/*
 * CommonClientTasks.h --
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFormat.h>
#include <camera.h>
#include <AstroCamera.h>

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
	void	binning(const astro::camera::Binning& binning);
	void	binning(const std::string& binning);
	void	exposuretime(double exposuretime);
	void	purpose(const astro::camera::Exposure::purpose_t purpose);
	void	purpose(const std::string& purposename);
	void	shutter(astro::camera::shutter_state shutter);
	Exposure	exposure() const;
	void	start();
	void	wait(int timeout = 60);
	void	available(int timeout = 60);
};

/**
 * \brief tasks related to the cooler
 *
 * This task sets up the cooler and waits for the temperature to be reached
 */
class CoolerTask {
	CoolerPrx&	_cooler;
	double	_absolute;
	bool	we_turned_cooler_on;
public:
	CoolerTask(CoolerPrx& cooler, double temperature);
	void	wait(int timeout = 300);
	~CoolerTask();
};

/**
 * \brief Task realted to the focuser
 *
 * The constructor of this task moves the focuser to a given position and
 * waits for the movement to complete
 */
class FocuserTask {
	FocuserPrx&	_focuser;
	int	_position;
	bool	we_started_focuser;
public:
	FocuserTask(FocuserPrx& focuser, int position);
	void	wait(int timeout = 60);
};

/**
 * \brief Task related to the filterwheel
 *
 * The constructor of this task moves 
 */
class FilterwheelTask {
	FilterWheelPrx&	_filterwheel;
	const std::string	_filtername;
	bool	we_started_filterwheel;
public:
	FilterwheelTask(FilterWheelPrx& filterwheel,
		const std::string& filtername);
	void	wait(int timeout = 60);
};

} // namespace snowstar
